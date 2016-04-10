#include "qtkapplicationparameters.h"
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <qcoreapplication.h>

QtKApplicationParameters::QtKApplicationParameters(QObject *parent, QString appName) :
    QObject(parent)
{
    this->m_appName = appName;
}

void QtKApplicationParameters::saveParam(QString groupName, QString paramName, QString paramValue, quint16 order)
{
        QString pn;
        QMap<QString, QVariant>::const_iterator i;
        i = this->m_projectParameters.find(this->m_appName);
        if(i == this->m_projectParameters.end())
        //Creem la entrada per el projecte
        {
           this->m_projectParameters.insert(this->m_appName,this->m_appName);
        }
        pn.append(this->m_appName);
        pn.append(".");
        pn.append(groupName);
        pn.append(".");
        pn.append(paramName);

        if(order)
        {
            pn.append(".");
            pn.append(QString("%1").arg(order));
        }
        this->m_projectParameters.insert(pn,paramValue);
        qDebug() << "saveParam() " << paramName << ":" << paramValue;
}

QString QtKApplicationParameters::loadParam(QString groupName, QString paramName, quint16 order)
{
    QString r;
    QString pn;
    QVariant d;
    pn.append(this->m_appName);
    pn.append(".");
    pn.append(groupName);
    pn.append(".");
    pn.append(paramName);
    if(order)
    {
        pn.append(".");
        pn.append(QString("%1").arg(order));
    }
    d = this->m_projectParameters.value(pn,QVariant(QString("void")));
    r = d.toString();
    qDebug() << "loadParam() " << paramName << ":" << r;
    return r;
}

bool QtKApplicationParameters::fileLoad(bool showAlerts)
{
        QString fileName = this->m_appName;
        fileName.append(AP_FILE_EXTENSION);
#ifndef ANDROID_PLATFORM
		fileName.prepend(qApp->applicationDirPath()+"/");
#endif

        QFile f(fileName);
        QByteArray fd;
        f.open(QIODevice::ReadOnly);
        if(f.error())
        {
              qDebug() << "fileLoad() Error: " << f.error() << ", " << f.errorString();
        }

        fd = f.readAll();
        f.close();

        QJsonDocument doc;
        QJsonParseError err;
        QJsonObject obj;

        doc = QJsonDocument::fromJson(fd,&err);

        if(err.error !=  QJsonParseError::NoError)
        {
            if(showAlerts)
            {
                qDebug() << "Error en el archivo de configuración: " << err.errorString().toLatin1().data() << "posición: " << err.offset;
            }

            emit applicationParametersError();
            return 1;
         }

         obj = doc.object();

         this->m_projectParameters.clear();
         this->m_projectParameters = obj.toVariantMap();
#ifdef AP_DEBUG_TRACES_ON
         qDebug() << "fileLoad(): " << this->m_projectParameters;
#endif
         if(this->m_projectParameters.contains(this->m_appName))
         {
            emit applicationParametersLoaded();
         }
#ifdef AP_DEBUG_TRACES_ON
         qDebug() << "fileLoad(END)";
#endif
         return 0; //Tot ok.
}

bool QtKApplicationParameters::fileSave()
{
	    QDateTime time =  QDateTime::currentDateTime();
        QString fileName = this->m_appName;
        fileName.append(AP_FILE_EXTENSION);
#ifndef ANDROID_PLATFORM
		fileName.prepend(qApp->applicationDirPath()+"/");
#endif

        saveParam(QString("Common"), QString("LastSave"),time.toString("dd.MM.yyyy - hh:mm:ss.zzz"), 0);

        QJsonDocument doc;
        QJsonObject obj;
        obj = QJsonObject::fromVariantMap(this->m_projectParameters);

        //qDebug() << "fileSave(): " << obj;
        doc.setObject(obj);

        QFile f(fileName);
        f.open(QIODevice::WriteOnly);
        if(f.error())
        {
              qDebug() << "fileSave() Error: " << f.error() << ", " << f.errorString();
        }

        f.write(doc.toJson());
        f.close();

        qDebug() << "fileSave(END)";
        return 0;
}
