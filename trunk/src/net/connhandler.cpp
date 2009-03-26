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

#include "net/server.h"
#include "net/connhandler.h"
#include "net/sockbuff.h"
#include "json/json.h"


/**************************************************************************************
*   \brief  Constructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SConnHandler::SConnHandler() : 
    pServer(NULL),
    clientSocket(-1),
    clientInput(NULL),
    clientOutput(NULL)
{
}

/**************************************************************************************
*   \brief  Destructor
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
SConnHandler::~SConnHandler()
{
    Reset();
}

/**************************************************************************************
*   \brief  Initialises the client.
*
*   \param  SServer *pServer    Parent server initialising this client.
*   \param  int sock            The socket for the client.
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
void SConnHandler::Init(SServer *server, int sock)
{
    pServer                 = server;
    clientSocket            = sock;
    SSocketBuff * strbuff   = new SSocketBuff(sock);
    clientInput             = new std::istream(strbuff);
    clientOutput            = new std::ostream(strbuff);
}

/**************************************************************************************
*   \brief  Clears input and output streams.
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
void SConnHandler::Reset()
{
    if (clientInput != NULL)
    {
        delete clientInput;
        clientInput = NULL;
    }

    if (clientOutput != NULL)
    {
        delete clientOutput;
        clientOutput = NULL;
    }

    if (clientSocket >= 0)
    {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }
}

/**************************************************************************************
*   \brief  Stops the client
*
*   \version
*       - Sri Panyam  10/02/2009
*         Created
**************************************************************************************/
int SConnHandler::RealStop()
{
    clientStopped = true;
    if (clientSocket >= 0)
    {
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }

    return 0;
}

