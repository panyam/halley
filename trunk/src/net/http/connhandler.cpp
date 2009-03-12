
//*****************************************************************************
/*!
 *  \file   client.cpp
 *
 *  \brief
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created
 *
 *****************************************************************************/

#include "connhandler.h"
#include "eds/http/request.h"
#include "eds/http/response.h"
#include "json/json.h"
#include <string.h>

/**************************************************************************************
*   \brief  Constructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SHttpConnHandler::SHttpConnHandler()
{
}

/**************************************************************************************
*   \brief  Destructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SHttpConnHandler::~SHttpConnHandler()
{
}

/**************************************************************************************
*   \brief  Registers a reqeust handler.
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
void SHttpConnHandler::RegisterRequestHandler(SHttpRequestHandler *pHandler,
                                              SHandlerType handlerType,
                                              int index)
{
    handlers[handlerType].push_back(pHandler);
}

/**************************************************************************************
*   \brief  Deregisters a reqeust handler.
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SHttpRequestHandler *SHttpConnHandler::UnRegisterRequestHandler(SHandlerType handlerType, int index)
{
    SHttpRequestHandler *out = handlers[handlerType].front();
    handlers[handlerType].pop_front();
    return out;
}

//*****************************************************************************
/*!
 *  \brief  Main client handle thread function
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
int SHttpConnHandler::Run()
{
    // Ignore SIGPIPE' so we can ignore error's 
    // thrown by out of band data
    signal(SIGPIPE, SIG_IGN);

    // disable naggle first!
    int nodelay = 1;
    if (setsockopt(clientSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        std::cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " 
             << strerror(errno) << std::endl << std::endl;
        return errno;
    }

    if (setsockopt(clientSocket, SOL_SOCKET, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        std::cerr << "ERROR: Could not set TCP_NODELAY on client socket [" << errno << "]: "
                  << strerror(errno) << std::endl << std::endl;
        return errno;
    }

    if (setsockopt(clientSocket, SOL_TCP, TCP_NODELAY, (char *)&nodelay, sizeof(nodelay)) != 0)
    {
        std::cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " 
             << strerror(errno) << std::endl << std::endl;
        return -errno;
    }

    //
    // ok here we listen to requests, process them and send the responses
    //
    // but we have couple of things in mind, it has to support multipart
    // messages without crazy cpu-hogging sleeps, ie it has to be
    // asynchronous
    //
    while (clientInput && clientOutput && 
           !(clientInput->bad() || clientInput->eof() ||
               clientOutput->bad() || clientOutput->eof()))
    {
        SHttpRequest  httpRequest;
        SHttpResponse httpResponse;

        // read the request
        httpRequest.ReadFromStream(*clientInput);

        // get it through the input handlers 
        for (HandlerList::iterator iter = handlers[INPUT_HANDLER].begin();
                    iter != handlers[INPUT_HANDLER].end(); ++iter)
        {
            if ((*iter)->ProcessRequest(httpRequest, httpResponse))
                break ;
        }

        // get it through the request handlers
        for (HandlerList::iterator iter = handlers[REQUEST_HANDLER].begin();
                    iter != handlers[REQUEST_HANDLER].end(); ++iter)
        {
            if ((*iter)->ProcessRequest(httpRequest, httpResponse))
                break ;
        }

        // now get it through the output handlers
        for (HandlerList::iterator iter = handlers[OUTPUT_HANDLER].begin();
                    iter != handlers[OUTPUT_HANDLER].end(); ++iter)
        {
            if ((*iter)->ProcessResponse(httpRequest, httpResponse))
                break ;
        }

        // write the headers to the output
        // httpResponse.WriteHeaders(*clientOutput);
        // now write the body
        // see if it is multipart

        // finally write the response
        httpResponse.WriteToStream(*clientOutput);

        // now write the multipart messages if any
        std::string connHeader = httpResponse.Headers().Header("Connection");
        if (strcasecmp(connHeader.c_str(), "close") == 0)
        {
            Reset();
        }
    }

    // indicate end of handling
    // pServer->

    // if (httpResponse.CloseConnection()) clientOutput->close();

    // TODO: See where to do this?
    // Reset();

    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Tries to open a file.
 *
 *  \version
 *      - Sri Panyam      10/02/2009
 *        Created.
 *
 *****************************************************************************/
FILE *SHttpConnHandler::OpenFile(const char *filename, const char *mode, std::string &errormsg)
{
    errormsg    = "Unknown Error";

    FILE *fptr  = fopen(filename, mode);
    if (fptr == NULL)
    {
        switch (errno)
        {
        case EEXIST:
            errormsg = "pathname already exists and O_CREAT and O_EXCL were used.";
            break;
        case EISDIR:
            errormsg = "pathname refers to a directory and the access "
                       "requested involved writing (that is, O_WRONLY "
                       "or O_RDWR is set).";
            break ;
        case EACCES:
            errormsg = "The requested access to the file is not allowed, or "
                       "one  of the directories  in  pathname did not allow "
                       "search (execute) permission, or the file did not "
                       "exist yet and write access to the parent directory "
                       "is not allowed.";
            break ;
        case ENAMETOOLONG:
            errormsg = "pathname was too long.";
            break ;
        case ENOENT:
            errormsg = "O_CREAT  is  not  set  and the named file does not "
                       "exist.  Or, a directory component in pathname does "
                       "not exist or is a dangling symbolic link.";
            break ;
        case ENOTDIR:
            errormsg = "A component  used as a directory in pathname is not, "
                       "in fact, a directory, or O_DIRECTORY was specified "
                       "and pathname was not a directory.";
            break ;
        case ENXIO:
            errormsg = "O_NONBLOCK  |  O_WRONLY  is set, the named file is a "
                       "FIFO and no process has the file open for reading.  "
                       "Or, the file is a device special file and no "
                       "corresponding device exists.";
            break ;
        case ENODEV:
            errormsg = "pathname  refers  to  a device special file and no "
                       "corresponding device exists.  (This is a Linux kernel "
                       "bug - in this situation ENXIO must be returned.)";
            break ;
        case EROFS:
            errormsg = "pathname  refers  to  a file on a read-only "
                       "filesystem and write access was requested.";
            break ;
        case ETXTBSY:
            errormsg =  "pathname refers to an executable image which is "
                        "currently being executed and write access was requested.";
            break ;
        case EFAULT:
            errormsg =  "pathname points outside your accessible address space.";
            break;
        case ELOOP:
            errormsg =  "Too many symbolic links were encountered in resolving "
                        "pathname, or O_NOFOLLOW was specified but pathname "
                        "was a symbolic link.";
            break ;
        case ENOSPC:
            errormsg =  "pathname was to be created but the  device  containing "
                        "pathname has no room for the new file.";
            break ;
        case ENOMEM:
            errormsg =  "Insufficient kernel memory was available.";
            break ;
        case EMFILE:
            errormsg =  "The process already has the maximum number of files open.";
            break ;
        case ENFILE:
            errormsg =  "The  limit  on  the total number of files open on the system "
                        "has been reached.";
            break ;
        }
    }

    return fptr;
}

