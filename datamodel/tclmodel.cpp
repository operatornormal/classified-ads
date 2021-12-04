/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2021.

  This file is part of Classified Ads.

  Classified Ads is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  Classified Ads is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with Classified Ads; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "tclmodel.h"
#include "tclprogram.h"
#include "../log.h"
#include "../util/hash.h"
#include "../util/ungzip.h"
#include "model.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QFileInfo>
#ifdef WIN32
#include <QCoreApplication>
#endif

TclModel::TclModel(MController *aController,
                   MModelProtocolInterface &aModel)
    :     iController(*aController),
          iModel(aModel) {
    LOG_STR("TclModel::TclModel()") ;
    connect(this,
            SIGNAL(  error(MController::CAErrorSituation,
                           const QString&) ),
            aController,
            SLOT(handleError(MController::CAErrorSituation,
                             const QString&)),
            Qt::QueuedConnection ) ;
    installExamplePrograms() ; 
}



TclModel::~TclModel() {
    LOG_STR("TclModel::~TclModel()") ;
}


Hash TclModel::locallyStoreTclProgram(const TclProgram& aProgram,
                                      const Hash& aPreviousFingerPrint ) {

    TclProgram possiblyExistingProgam ( tclProgramByFingerPrint ( aPreviousFingerPrint ) ) ;
    bool ret(false) ;
    QLOG_STR("Storing TCL program with hash " + 
             aProgram.iFingerPrint.toString()) ; 
    if ( aPreviousFingerPrint != KNullHash &&
            possiblyExistingProgam.iFingerPrint != KNullHash ) {
        // there is previous version of the same program
        QSqlQuery query(iModel.dataBaseConnection());
        ret = query.prepare ("update tclprogs set prog_name = :progname, "
                             "prog_text=:progtext, "
                             "time_modified=:time_modified, "
                             "hash1 = :new_hash1, hash2=:new_hash2, "
                             "hash3= :new_hash3, hash4=:new_hash4, "
                             "hash5=:new_hash5 "
                             "where hash1 = :hash1 and hash2 = :hash2 "
                             "and hash3 = :hash3 and hash4 = :hash4 "
                             "and hash5 = :hash5" ) ;
        if ( ret ) {
            query.bindValue(":hash1", aPreviousFingerPrint.iHash160bits[0]);
            query.bindValue(":hash2", aPreviousFingerPrint.iHash160bits[1]);
            query.bindValue(":hash3", aPreviousFingerPrint.iHash160bits[2]);
            query.bindValue(":hash4", aPreviousFingerPrint.iHash160bits[3]);
            query.bindValue(":hash5", aPreviousFingerPrint.iHash160bits[4]);
            query.bindValue(":progname", aProgram.programName());
            QByteArray programTextCompressed (qCompress(aProgram.programText().toUtf8())) ;
            query.bindValue(":progtext", programTextCompressed);
            query.bindValue(":time_modified", QDateTime::currentDateTimeUtc().toTime_t());
            query.bindValue(":new_hash1", aProgram.iFingerPrint.iHash160bits[0]);
            query.bindValue(":new_hash2", aProgram.iFingerPrint.iHash160bits[1]);
            query.bindValue(":new_hash3", aProgram.iFingerPrint.iHash160bits[2]);
            query.bindValue(":new_hash4", aProgram.iFingerPrint.iHash160bits[3]);
            query.bindValue(":new_hash5", aProgram.iFingerPrint.iHash160bits[4]);
        }
        ret = query.exec() ;
        if ( !ret ) {
            LOG_STR2("Error while tcl program update %s", qPrintable(query.lastError().text())) ;
            emit error(MController::DbTransactionError, query.lastError().text()) ;
        } else {
            QLOG_STR("locallyStoreTclProgram update " + QString::number(ret)) ;
        }
    } else {
        // there were no previous version of the program

      QSqlQuery query3(iModel.dataBaseConnection()) ;
        ret = query3.prepare ( "insert into tclprogs ("
                               "hash1,hash2,hash3,hash4,hash5,"
                               "prog_name,prog_text,time_modified) "
                               "values ( "
                               ":hash1,:hash2,:hash3,:hash4,:hash5,"
                               ":progname,:progtext,:time_modified)") ;
        if ( !ret ) {
            emit error(MController::DbTransactionError, query3.lastError().text()) ;
        } else {
            query3.bindValue(":progname", aProgram.programName());
            QByteArray programTextCompressed (qCompress(aProgram.programText().toUtf8())) ;
            query3.bindValue(":progtext", programTextCompressed);
            query3.bindValue(":time_modified", QDateTime::currentDateTimeUtc().toTime_t());
            query3.bindValue(":hash1", aProgram.iFingerPrint.iHash160bits[0]);
            query3.bindValue(":hash2", aProgram.iFingerPrint.iHash160bits[1]);
            query3.bindValue(":hash3", aProgram.iFingerPrint.iHash160bits[2]);
            query3.bindValue(":hash4", aProgram.iFingerPrint.iHash160bits[3]);
            query3.bindValue(":hash5", aProgram.iFingerPrint.iHash160bits[4]);
            ret = query3.exec() ;
            QLOG_STR("locallyStoreTclProgram insert " + QString::number(ret)) ;
        }
        if ( !ret ) {
            QLOG_STR("locallyStoreTclProgram " + query3.lastError().text() ) ;
            emit error(MController::DbTransactionError, query3.lastError().text()) ;
        }
    }
    QLOG_STR("Going to return hash " +
             Hash(ret == true ? aProgram.iFingerPrint : KNullHash).toString());
    return ret == true ? aProgram.iFingerPrint : KNullHash ;

}


TclProgram TclModel::tclProgramByFingerPrint(const Hash& aFingerPrint) {
    LOG_STR("TclprogramModel::tclprogramByFingerPrint()") ;
    TclProgram retval ;
    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    ret = query.prepare ("select prog_name,prog_text,prog_settings,time_modified from tclprogs where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) && !query.isNull(1) ) {
            retval.iFingerPrint = aFingerPrint ;
            retval.iTimeOfPublish = query.value(3).toUInt() ;
            retval.setProgramName(query.value(0).toString()) ;
            const QByteArray programTextCompressed ( query.value(1).toByteArray() );
            retval.setProgramText(qUncompress(programTextCompressed)) ;
        }
    }
    return retval ;
}

QMap<QString, Hash> TclModel::getListOfTclPrograms() {
    QSqlQuery query(iModel.dataBaseConnection());
    QMap<QString, Hash> retval ;
    bool ret ;
    ret = query.prepare ("select hash1,hash2,hash3,hash4,hash5,"
                         "prog_name from tclprogs order by time_modified" ) ;
    if ( ret ) {
        ret = query.exec() ;
        if ( !ret ) {
            QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
            emit error(MController::DbTransactionError, query.lastError().text()) ;
        } else {
            while ( query.next() ) {
                quint32 hash1 = query.value(0).toUInt() ;
                quint32 hash2 = query.value(1).toUInt() ;
                quint32 hash3 = query.value(2).toUInt() ;
                quint32 hash4 = query.value(3).toUInt() ;
                quint32 hash5 = query.value(4).toUInt() ;
                QString programName ( QString::fromUtf8(query.value(5).toByteArray()) ) ;
                QLOG_STR("Appending to list of TCL programs in storage: " + programName) ;
                retval.insert(programName , Hash ( hash1,hash2,hash3,hash4,hash5) );
            }
        }
    } else {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    }
    return retval ;
}

bool TclModel::discardTclProgram(const Hash& aFingerPrint) {
    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    ret = query.prepare ("delete from tclprogs where hash1=:hash1 and hash2=:hash2 and "
                         "hash3=:hash3 and hash4=:hash4 and hash5=:hash5" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aFingerPrint.iHash160bits[0]);
        query.bindValue(":hash2", aFingerPrint.iHash160bits[1]);
        query.bindValue(":hash3", aFingerPrint.iHash160bits[2]);
        query.bindValue(":hash4", aFingerPrint.iHash160bits[3]);
        query.bindValue(":hash5", aFingerPrint.iHash160bits[4]);
        ret = query.exec() ;
    }
    QLOG_STR("TclModel::discardTclProgram " + QString::number(ret)) ;
    return ret ;
}

QString TclModel::storeTCLProgLocalData(const Hash& aProgram,
                                        const QByteArray& aData) {
    QLOG_STR("About to store data len =" + QString::number(aData.size())) ;  
    QLOG_STR("Storing TCL program data, prog hash = " + 
             aProgram.toString()) ; 
    // check if program is there:
    TclProgram  prog = tclProgramByFingerPrint(aProgram)  ;
    if ( prog.iFingerPrint == KNullHash ) {
        return "No program found" ;
    } else {
        QSqlQuery query(iModel.dataBaseConnection());
        bool ret ;
        ret = query.prepare ("update tclprogs set prog_data = :d where hash1=:hash1 and hash2=:hash2 and "
                             "hash3=:hash3 and hash4=:hash4 and hash5=:hash5" ) ;
        if ( ret ) {
            query.bindValue(":hash1", aProgram.iHash160bits[0]);
            query.bindValue(":hash2", aProgram.iHash160bits[1]);
            query.bindValue(":hash3", aProgram.iHash160bits[2]);
            query.bindValue(":hash4", aProgram.iHash160bits[3]);
            query.bindValue(":hash5", aProgram.iHash160bits[4]);
           QLOG_STR("About to store data " + 
                     QString(aData)) ; 
            if ( aData.size() > 0 ) {
                QByteArray programDataCompressed (qCompress(aData)) ;
                query.bindValue(":d", programDataCompressed);
            } else {
                query.bindValue(":d", QString()); // null value
            }
            ret = query.exec() ;
            if ( query.numRowsAffected() == 0 ) {
                return ("Data not saved, is program stored?") ; 
            }
        }
        QLOG_STR("TclModel::storeTCLProgLocalData " + QString::number(ret)) ;
        if ( ret == false ) {
            return query.lastError().text();
        } 
    }
    return QString::null ; 
}

QByteArray TclModel::retrieveTCLProgLocalData(const Hash& aProgram) {
    QByteArray retval ; 
    QSqlQuery query(iModel.dataBaseConnection());
    bool ret ;
    QLOG_STR("Retrieving TCL program data, prog hash = " + 
             aProgram.toString()) ; 
    ret = query.prepare ("select prog_data from tclprogs where hash1 = :hash1 and hash2 = :hash2 and hash3 = :hash3 and hash4 = :hash4 and hash5 = :hash5" ) ;
    if ( ret ) {
        query.bindValue(":hash1", aProgram.iHash160bits[0]);
        query.bindValue(":hash2", aProgram.iHash160bits[1]);
        query.bindValue(":hash3", aProgram.iHash160bits[2]);
        query.bindValue(":hash4", aProgram.iHash160bits[3]);
        query.bindValue(":hash5", aProgram.iHash160bits[4]);
    }
    ret = query.exec() ;
    if ( !ret ) {
        QLOG_STR(query.lastError().text() + " "+ __FILE__ + QString::number(__LINE__)) ;
        emit error(MController::DbTransactionError, query.lastError().text()) ;
    } else {
        if ( query.next() && !query.isNull(0) ) {
            const QByteArray programDataCompressed ( query.value(0).toByteArray() );
            retval.append(qUncompress(programDataCompressed)) ;
        }
    }
    return retval ;
}

void TclModel::installExamplePrograms() {
    if ( getListOfTclPrograms().isEmpty() ) {
        // this is our first run or user deleted all example programs.
        // bring 'em back:
#ifdef WIN32
        // in windows try fetch examples from path relative to
        // the executable, see file windows/nsis-installer.nsi for details
        QString examplesDirName(QCoreApplication::applicationDirPath()) ;
        examplesDirName.append(QDir::separator()) ;
        examplesDirName.append("examples") ; 
        QDir examplesDir (examplesDirName) ;
#else
        // in unix this path appears in classified-ads.pro and 
        // is used by "make install" phase.
        QDir examplesDir ("/usr/share/doc/classified-ads/examples") ;
        if ( !examplesDir.exists() ) {
	  // some linux distributions want version number inside path,
	  // like "/usr/share/doc/classified-ads-v2.0" so
	  // lets try find one with version number:
	  QDir directoryEnumerator("/usr/share/doc") ;
	  QStringList filePattern ;
	  filePattern << "classified-ads*" ;

	  QFileInfoList list = directoryEnumerator.entryInfoList(filePattern,
								 QDir::Dirs);
	  for (int i = 0; i < list.size(); ++i) {
	    QFileInfo fileInfo = list.at(i);
	    QLOG_STR("Selecting example files dir " + fileInfo.fileName()) ;
	    examplesDir.setPath(fileInfo.absoluteFilePath() +
				"/examples") ;
	    // break out with first match ; idea is that no multiple
	    // versions of this sw can be installed at once ; first
	    // match is only match
	    break ; 
	  }
        }
#endif
        if ( !examplesDir.exists() ) {
            return ; // no examples, obviously
        }
        examplesDir.setFilter(QDir::Files | QDir::Readable) ;
        QStringList fileTypes ; 
        // debian installation insists on compression of example files ; we
        // need to apply un-gzip here to make examples usable again:
        fileTypes << "*.tcl" << "*.tcl.gz" ;
        const QStringList exampleFiles ( examplesDir.entryList(fileTypes) ) ; 
        foreach ( const QString& exampleFileName, exampleFiles ) {
            QLOG_STR(exampleFileName) ;
            QFile exampleFile(examplesDir.filePath(exampleFileName));
            if (exampleFile.open(QIODevice::ReadOnly)) {
                QByteArray exampleContents ( exampleFile.readAll() ) ; 
                if ( exampleContents.length() > 0 ) {
                    TclProgram p ;
                    if ( exampleFileName.toLower().endsWith (".tcl.gz" ) ) {
                        QByteArray uncompressed ;
                        bool success ; 
                        uncompressed.append ( UnGZip::unGZip(exampleContents,
                                                             &success) ) ;
                        if ( !success || ( uncompressed.length() < 1 ) ) {
                            continue ; 
                        } else {
                            exampleContents.clear() ; 
                            exampleContents.append(uncompressed) ; 
                        }
                    }
                    p.setProgramText(exampleContents) ; 
                    p.setProgramName(exampleFileName) ; 
                    const QFileInfo info ( exampleFile ) ; 
                    p.iTimeOfPublish = info.lastModified().toTime_t() ; 
                    locallyStoreTclProgram(p) ; 
                }
            }
        }
    }
}
