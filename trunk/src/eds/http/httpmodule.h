//*****************************************************************************
/*!
 *  \file   httpmodule.h
 *
 *  \brief  Everything requried to write a handler module.
 *
 *  \version
 *      - S Panyam      04/03/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SHTTP_HANDLER_MODULE_H_
#define _SHTTP_HANDLER_MODULE_H_

#include <queue>
#include <functional>
#include "eds/stage.h"
#include "httpfwd.h"

//! Maintains request specific module data.
class SHttpModuleData
{
public:
    //! Creates a module data object.
    SHttpModuleData() { Reset(); }

    //! Virtual destructor
    virtual ~SHttpModuleData();

    //! Resets the module data
    virtual void Reset()
    {
        lastBP = lastBPSent = 0;
        processing = false;
    }

public:
    //! Gets the next body part that can be processed (if any).
    virtual void PutBodyPart(SBodyPart *pCurrPart);

    //! Gets the next body part if it is processable
    virtual SBodyPart *NextBodyPart();

    //! Short cut for put and next calls
    virtual SBodyPart *PutAndGetBodyPart(SBodyPart *pCurrPart);

public:
    //! last BP processed
    BPIndexType lastBP;

    //! Last BP sent to the next module, this is so that we may consume
    // modules or merge modules in which case we dont want "holes"
    // appearing in body parts recieved by modules further along the chain.
    //
    // BodyPart Ordering
    // =================
    //
    // Lowerlevel modules will generate bodyparts at will.  But the ordering of
    // these body parts must not be ignored.  There must be mechanisms to allow
    // orderings of BodyPart processing.  For example in the following sequence
    // of body parts, we may have:
    //
    // 1 2 3 4 5 6
    //
    // 1 2 3 in one sequence, 5 6 in another sequence and with it being ok to
    // process 4 at "any" time.  ie we can have the following processing order:
    //
    // 1 5 2 4 6 or
    // 5 1 2 3 6 4
    //
    // but the following are invalid:
    //
    // 2 1 3        (2 processed before 1)
    // 4 5 1 6 3 2  (3 being processed before 2)
    //
    // So how can modules specify ordering because once a module sends out a
    // body part, it has no control over it.  Essentially another module may
    // already process the body part and delete the structure, so we cannot
    // even hold a pointer to it.  How about ref counted smart pointers (brings
    // issues of too much lock contention in changins refcounts)?
    //
    // How about a module level solution?  The problem only really arises if
    // there is a "small" gap between when body part messages are picked up by
    // each stage.  So if there is a large enough gap, it can be ensured that
    // parent body parts will be processed first.  But this is not right to
    // depend on the "large" gap assumption especially in a server that may
    // stream financial quotes with high burst rates.
    //
    // How about indices in body parts?  So a module that recieves them will
    // simply queue them in order and process them as it sees it?
    //
    // it can be 8 byte unsigned long long - which would be ok for 18 billion
    // billion messages - even at a body part every millisecond this would be
    // 18 trillion seconds (571000 years) worth before we run out.  And this is
    // only one for a single request, if we maintain a counter per request.
    //
    // This means that each body part HAS to be processed sequentially.  So for
    // a request, if we had multiple modules sending out body parts, the
    // processing of these body parts in the later stage should not have
    // volatility in processing requirements based on the content - should be
    // roughly "same" - one way to handle this may be by mandating that a
    // NewBodyPart be called only after all the data that goes in the body part
    // is available - or that even if it is called, a new counter be assigned only
    // when it is needed instead of at creation time.

    BPIndexType lastBPSent;

    //! true if this module is currently processing this body part.
    bool        processing;

private:
    //! Comparator for the body parts
    class SBPComparer
    {
    public:
        bool operator()(const SBodyPart *a, const SBodyPart *b) const;
    };

private:
    //! A priority of BPs that havent yet been processed.
    std::priority_queue<SBodyPart *, std::vector<SBodyPart *>, SBPComparer>    bodyParts;
};

// Stores the handler state data
class SHttpHandlerData
{
public:
    //! Create a state object
    SHttpHandlerData(SConnection *pConn);

    //! Destroy the state
    virtual ~SHttpHandlerData();

    //! gets the version of this handler data
    virtual int Version() { return 1; }

    //! Resets all modules when a new request begins.
    virtual void ResetModuleData();

    //! Gets the module specific data - so each module can keep its own
    // state regarding this connection
    virtual SHttpModuleData *GetModuleData(SHttpModule *pModule, bool create = false);

    //! Sets the module specific data - so each module can keep its own
    // state regarding this connection
    virtual void SetModuleData(SHttpModule *pModule, SHttpModuleData *pData);

    //! Adds a new request to the queue.
    virtual void AddRequest(SHttpRequest *pReq) { requests.push_back(pReq); }

    //! Destroys the current request
    virtual void DestroyRequest();

    //! Current request being handled
    inline SHttpRequest *Request() { return requests.empty() ? NULL : requests.front(); }

public:
    //! The connection object
    SConnection *   pConnection;

protected:
    typedef std::pair<SHttpModule *, SHttpModuleData *> ModuleData;

    //! Hold module specific data
    std::list<ModuleData *>   moduleData;

    //! List of requests being handled
    std::list<SHttpRequest *>   requests;
};

//!
// These are the modules that people will be writing and extending to
// extend the server.
//
// Essentially the idea is a module will do some processing and send out
// the data to the next module.  The nature of the data is arbitrary (in
// the apache tradition!).
//
class SHttpModule
{
public:
    //! Creates a new http module
    SHttpModule(SHttpModule *pNext) : pNextModule(pNext) { }

    //! Empty virtual destructor to safeguard against some compilers
    virtual ~SHttpModule() { }

    //! Removes/Destroyes any module specific data it may have stored
    virtual void RemoveModuleData(SHttpModuleData *pData) { if (pData) delete pData; }

    //! Called to handle input data from another module
    virtual void ProcessInput(SHttpHandlerData *    pHandlerData,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart) { }

    //! Called to handle output data from another module
    virtual void ProcessOutput(SHttpHandlerData *   pHandlerData,
                               SHttpHandlerStage *  pStage,
                               SBodyPart *          pBodyPart) { }

    //! Creates new module data if necessary
    virtual SHttpModuleData *CreateModuleData(SHttpHandlerData *pHandlerData);

protected:
    //! Send a body part to another module
    void SendBodyPartToModule(SConnection *         pConn,
                              SHttpHandlerStage *   pStage,
                              SBodyPart *           pBodyPart,
                              SHttpModuleData *     pModData,
                              SHttpModule *         pModule);

protected:
    //! The next module in the chain
    SHttpModule *       pNextModule;
};

#endif

