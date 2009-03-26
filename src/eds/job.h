//*****************************************************************************
/*!
 *  \file   job.h
 *
 *  \brief
 *  A job that is handled in different stages.
 *
 *  \version
 *      - Sri Panyam      20/02/2009
 *        Created
 *
 *****************************************************************************/

#ifndef _SEDS_JOB_H_
#define _SEDS_JOB_H_

#include "eds/fwd.h"

//*****************************************************************************
/*!
 *  \class  SJob
 *
 *  \brief A job that is handled in different stages.
 *
 *****************************************************************************/
class SJob
{
public:
    //! Destroys the connection object
    SJob();

    //! Destroys the connection object
    virtual ~SJob() { }

    //! Get the stage specific data for this connection
    void *  GetStageData(SStage *pStage);

    //! Set the stage specific data for this connection
    void    SetStageData(SStage *pStage, void * data);

private:
    //! Connection specific data that handlers can manipulate to their will.
    std::vector<void *>     stageData;
};


#endif

