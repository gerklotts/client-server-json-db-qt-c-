#include "myserver.h"

myserver::myserver(){}

myserver::~myserver(){}

void myserver::startServer()
{
    if(this->listen(QHostAddress::Any,5555))
    {
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("D:\\workers.db");
        if(db.open())
        {
            qDebug()<<"Listening and db is opened";
        }
        else
        {
            qDebug()<<"db is not opened";
        }
    }
    else
    {
        qDebug()<<"Not listening";
    }
}

void myserver::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

        connect(socket, SIGNAL(readyRead()),this,SLOT(sockReady()));
        connect(socket,SIGNAL(disconnected()),this,SLOT(sockDisc()));

    qDebug()<<socketDescriptor<<"Client connected";

    socket->write("{\"type\":\"connect\",\"status\":\"yes\"}");
    qDebug()<<"Send client connect status - YES";
}

void myserver::sockReady()
{
    Data = socket->readAll();
    qDebug()<<"Data: "<<Data;

    doc = QJsonDocument::fromJson(Data, &docError);

    if(docError.errorString().toInt()==QJsonParseError::NoError)
    {
        if((doc.object().value("type").toString()=="recSize")&&(doc.object().value("resp")=="workers"))
        {

            itog = "{\"type\":\"resultSelect\",\"result\":[";

            if(db.open())
            {
                QSqlQuery *query = new QSqlQuery(db);
                if(query->exec("SELECT name FROM listworkers"))
                {
                    while(query->next())
                    {
                        itog.append("{\"name\":\""+query->value(0).toString()+"\"},");
                    }

                    itog.remove(itog.length()-1,1);
                }
                else
                {
                    qDebug()<<"Query not success";
                }

                delete query;
            }
            itog.append("]}");

            socket->write("{\"type\":\"size\",\"resp\":\"workers\",\"length\":"+QByteArray::number(itog.size())+"}");
            socket->waitForBytesWritten(500);
        }
        else if((doc.object().value("type").toString()=="select")&&(doc.object().value("params")=="workers"))
        {
            socket->write(itog);
            qDebug()<<"Razmer soobsheniya: "<<socket->bytesToWrite();
            socket->waitForBytesWritten(500);
        }
    }
}

void myserver::sockDisc()
{
    qDebug()<<"Disconnect";
    socket->deleteLater();
}
