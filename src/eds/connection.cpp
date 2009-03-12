//*****************************************************************************
/*!
 *  \file   connection.cpp
 *
 *  \brief
 *
 *  A connection in an event driver server.
 *
 *  \version
 *      - Sri Panyam      18/02/2009
 *        Created
 *
 *****************************************************************************/

#include "connection.h"
#include "server.h"
#include "stage.h"
#include "handler.h"

/**************************************************************************************
*   \brief  Creates a new connection object and required members.
*
*   \version
*       - Sri Panyam  04/03/2009
*         Created
**************************************************************************************/
SConnection::SConnection(SEvServer *pSrv, int sock) : 
        pServer(pSrv),
        connSocket(sock),
        socketBuff(new SSocketBuff(sock)),
        clientOutput(new std::ostream(socketBuff))
{
}

/**************************************************************************************
*   \brief  Destroys the connection and associated data.
*
*   \version
*       - Sri Panyam  04/03/2009
*         Created
**************************************************************************************/
SConnection::~SConnection()
{
    delete socketBuff;
    delete clientOutput;
}

/**************************************************************************************
*   \brief  Closes the connection.
*
*   \version
*       - Sri Panyam  19/02/2009
*         Created
**************************************************************************************/
void SConnection::Close()
{
    pServer->ConnectionComplete(this);
}

