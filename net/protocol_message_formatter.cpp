/*     -*-C++-*- -*-coding: utf-8-unix;-*-
       Classified Ads is Copyright (c) Antti Järvinen 2013.

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
#ifdef WIN32
#define NOMINMAX
#include <WinSock2.h>
#endif
#include "protocol_message_formatter.h"
#include <time.h>
#ifndef WIN32
#include "arpa/inet.h"
#endif
#include "../util/jsonwrapper.h"
#include "node.h"
#include "connection.h"
#include "../log.h"
#include "../datamodel/const.h"
#include "../datamodel/searchmodel.h"
#include "../datamodel/voicecall.h"
#include "../datamodel/model.h"
#include "../datamodel/contentencryptionmodel.h"

QByteArray ProtocolMessageFormatter::nodeGreeting(const Node& aNode) {
    QByteArray retval ;
    unsigned char ch ;
    ch = KProtocolVersion ;// (unsigned char) PROTOCOL_VERSION ;
    retval.append((const char *)&ch, 1) ;
    QVariant v ( aNode.asQVariant()) ;

    QByteArray nodeRefAsJson ( JSonWrapper::serialize(v) ) ;
    QLOG_STR(nodeRefAsJson) ;
    QByteArray nodeRefAsCompressedJson (qCompress( nodeRefAsJson )) ;
    nodeRefAsJson.clear() ;
    quint32 nodeRefSize = nodeRefAsCompressedJson.size() ;
    quint32 nodeRefSizeNetworkByteorder = htonl(nodeRefSize + 1 + sizeof(quint32));
    retval.append((const char *)(&nodeRefSizeNetworkByteorder), sizeof(nodeRefSize)) ;
    retval.append(nodeRefAsCompressedJson) ;

    return retval ;
}

QByteArray ProtocolMessageFormatter::randomNumbers(void) {
    QByteArray retval ;
    unsigned char ch ;
    ch = KRandomNumbersPacket ;
    retval.append((const char *)&ch, 1) ;
    ch = 0 ;
    retval.append((const char *)&ch, 1) ; // placeholder for message length
    unsigned char numberOfBytesInMessage = 2 + ( rand() % 15 ) ;

    for ( unsigned char i = 0 ; i < numberOfBytesInMessage ; i++ ) {
        ch = ( rand() % 255 ) ;
        retval.append((const char *)&ch, 1) ;
    }
    // then count message len
    unsigned char messagelen = (unsigned char)(retval.length()) ;
    retval.replace(1,1, (const char *) &messagelen, 1) ;
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForBinaryBlob(const Hash& aHash) {
    QByteArray retval ;
    unsigned char ch ( KBinaryFileAtHash  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForPrivateMessages(const Hash& aHash) {
    QByteArray retval ;
    unsigned char ch ( KPrivMessagesAtHash  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForPrivateMessagesForProfile(const Hash& aHash,const quint32 aTimeOfOldestMsgToSend) {
    QByteArray retval ;
    unsigned char ch ( KPrivMessagesForProfile  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    quint32 tsInNetworkBO = htonl(aTimeOfOldestMsgToSend) ;
    retval.append((const char *)(&tsInNetworkBO), sizeof(quint32)) ;
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForProfileComments(const Hash& aCommentedProfileHash,
        const quint32 aTimeOfOldestMsgToSend) {
    QByteArray retval ;
    unsigned char ch ( KProfileCommentAtHash  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aCommentedProfileHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    quint32 tsInNetworkBO = htonl(aTimeOfOldestMsgToSend) ;
    retval.append((const char *)(&tsInNetworkBO), sizeof(quint32)) ;
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForUserProfileComment(const Hash& aIndividualCommentHash) {
    QByteArray retval ;
    unsigned char ch ( KSingleProfileCommentAtHash ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aIndividualCommentHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    return retval ;
}



QByteArray ProtocolMessageFormatter::requestForNodesAroundHash(const Hash& aHash) {
    QByteArray retval ;
    unsigned char ch ( KNodesAroundHash  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForUserProfile(const Hash& aHash) {
    QByteArray retval ;
    unsigned char ch ( KProfileAtHash  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForClassifiedAd(const Hash& aHash) {
    QByteArray retval ;
    unsigned char ch ( KClassifiedAdAtHash  ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHash.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::requestForAdsClassified(const Hash& aHashOfClassification,
        const quint32  aStartingTimestamp  ,
        const quint32  aEndingTimestamp ) {
    QByteArray retval ;
    unsigned char ch ( KAdsClassifiedAtHash ) ;
    retval.append((const char *)&ch, 1) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashNumberNetworkByteOrder (htonl(aHashOfClassification.iHash160bits[i])) ;
        retval.append((const char *)(&hashNumberNetworkByteOrder), sizeof(quint32)) ;
    }
    const quint32  startingTimestampNetworkBO(htonl(aStartingTimestamp)) ;
    const quint32  endingTimestampNetworkBO(htonl(aEndingTimestamp)) ;
    retval.append((const char *)(&startingTimestampNetworkBO), sizeof(quint32)) ;
    retval.append((const char *)(&endingTimestampNetworkBO), sizeof(quint32)) ;
    return retval ;
}

QByteArray ProtocolMessageFormatter::contentPublish(const unsigned char aContentMagicNumber,
        const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const QByteArray& aSigningKey,
        bool aIsContentEncrypted,
        bool aIsContentCompressed,
        quint32 aTimeStamp) {
    QByteArray retval ;
    return doContentSendOrPublish(retval,
                                  aContentMagicNumber,
                                  aContentHash,
                                  aContent,
                                  aSignature,
                                  aBangPath,
                                  aSigningKey,
                                  aIsContentEncrypted,
                                  aIsContentCompressed,
                                  aTimeStamp,
                                  true) ;
}

QByteArray ProtocolMessageFormatter::contentSend(const unsigned char aContentMagicNumber,
        const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QByteArray& aSigningKey,
        bool aIsContentEncrypted,
        bool aIsContentCompressed,
        quint32 aTimeStamp) {
    QList<quint32> aDummyBangPath ;
    QByteArray retval ;
    return doContentSendOrPublish(retval,
                                  aContentMagicNumber,
                                  aContentHash,
                                  aContent,
                                  aSignature,
                                  aDummyBangPath,
                                  aSigningKey,
                                  aIsContentEncrypted,
                                  aIsContentCompressed,
                                  aTimeStamp,
                                  false) ;
}

QByteArray ProtocolMessageFormatter::doContentSendOrPublish(QByteArray& retval,
        const unsigned char aContentMagicNumber,
        const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const QByteArray& aSigningKey,
        bool aIsContentEncrypted,
        bool aIsContentCompressed,
        quint32 aTimeStamp,
        bool aIsPublish) {

    // ok, lets decide here the layout of publish..
    // first magick number telling what it is.
    // then bangpath 5 items and if not all 5 are filled,
    // the remaining ones are filled with zeroes.

    // then content hash, 5*32 bytes

    // then length of the actual data, followed by the actual data
    // then length of the signature, followeds by the signature.
    // then length of the signingkey, followed by the key
    // then flags, one unsigned char.
    //  * if bit 1 is up, it means that content is encrypted.
    //  * if bit 2 is up, it means that content is compressed.
    // then time when content was published. For most of the content
    // the time of publish may be deduced from content itself but in some
    // case like the private profile the date of publish is hidden from
    // us -> this time practically acts as a version-number for the
    // content.
    LOG_STR2("content len = %d", (int)aContent.length()) ;
    LOG_STR2("signature len = %d", (int)aSignature.length()) ;
    if ( aIsPublish ) {
        for ( int i = 0 ; i < aBangPath.size() ; i++ ) {
            LOG_STR2("Bangpath contains %u", aBangPath[i]) ;
        }
    }
    unsigned char ch ;
    ch = aContentMagicNumber ;
    retval.append((const char *)&ch, 1) ;

    quint32 bPathNumberNetworkByteorder ;
    if ( aIsPublish ) {
        if ( aBangPath.size() >= 1 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[0]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 2 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[1]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 3 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[2]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 4 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[3]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 5 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[4]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
    }
    // now has 5 numbers as bangpath, then put hash
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aContentHash.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    // after hash, the size of content:
    quint32 lengthNetworkByteorder ( htonl( (quint32)(aContent.size()) ) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    // followed by content:
    retval.append(aContent) ;
    lengthNetworkByteorder = htonl( (quint32)(aSignature.size()) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    retval.append(aSignature) ;
    // followed by signing key

    lengthNetworkByteorder = htonl( (quint32)(aSigningKey.size()) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    retval.append(aSigningKey)  ;

    unsigned char flags ( 0 ) ;
    if ( aIsContentEncrypted ) {
        flags = flags | 1 ;
    }
    if ( aIsContentCompressed ) {
        flags = flags | 2 ;
    }
    retval.append((const char *)&flags, sizeof(unsigned char))  ;
    quint32 timestampNetworkBO ( htonl( aTimeStamp ) ) ;
#ifndef WIN32
#ifdef DEBUG
    time_t contentTime_T ( aTimeStamp ) ;
    char timeBuf[40] ;
    LOG_STR2("Content time of publish at send: %s", ctime_r(&contentTime_T,timeBuf)) ;
#endif
#endif
    retval.append((const char *)&timestampNetworkBO, sizeof(quint32))  ;
    // and that's all folks
    return retval ;
}


QByteArray ProtocolMessageFormatter::privMsgPublish(const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const Hash& aDestination,
        const Hash& aRecipient,
        quint32 aTimeStamp ) {


    for ( int i = 0 ; i < aBangPath.size() ; i++ ) {
        LOG_STR2("Bangpath if privmsg contains %u", aBangPath[i]) ;
    }

    unsigned char ch ( KPrivMessagePublish ) ;
    QByteArray retval ;
    retval.append((const char *)&ch, 1) ;

    quint32 bPathNumberNetworkByteorder ;

    if ( aBangPath.size() >= 1 ) {
        bPathNumberNetworkByteorder = htonl(aBangPath[0]);
    } else {
        bPathNumberNetworkByteorder = 0 ;
    }
    retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
    if ( aBangPath.size() >= 2 ) {
        bPathNumberNetworkByteorder = htonl(aBangPath[1]);
    } else {
        bPathNumberNetworkByteorder = 0 ;
    }
    retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
    if ( aBangPath.size() >= 3 ) {
        bPathNumberNetworkByteorder = htonl(aBangPath[2]);
    } else {
        bPathNumberNetworkByteorder = 0 ;
    }
    retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
    if ( aBangPath.size() >= 4 ) {
        bPathNumberNetworkByteorder = htonl(aBangPath[3]);
    } else {
        bPathNumberNetworkByteorder = 0 ;
    }
    retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
    if ( aBangPath.size() >= 5 ) {
        bPathNumberNetworkByteorder = htonl(aBangPath[4]);
    } else {
        bPathNumberNetworkByteorder = 0 ;
    }
    retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;

    // now has 5 numbers as bangpath, then put hash
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aContentHash.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    // after hash, the size of content:
    quint32 lengthNetworkByteorder ( htonl( (quint32)(aContent.size()) ) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    // followed by content:
    retval.append(aContent) ;
    lengthNetworkByteorder = htonl( (quint32)(aSignature.size()) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    retval.append(aSignature) ;

    quint32 timestampNetworkBO ( htonl( aTimeStamp ) ) ;
#ifndef WIN32
#ifdef DEBUG
    time_t contentTime_T ( aTimeStamp ) ;
    char timebuf[40] ;
    LOG_STR2("Private message time of publish at send: %s", ctime_r(&contentTime_T,timebuf)) ;
#endif
#endif
    retval.append((const char *)&timestampNetworkBO, sizeof(quint32))  ;

    // then destination node:
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aDestination.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    // then recipient:
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aRecipient.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::privMsgSend(const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const Hash& aDestination,
        const Hash& aRecipient,
        quint32 aTimeStamp ) {


    unsigned char ch ( KPrivMessageSend ) ;
    QByteArray retval ;
    retval.append((const char *)&ch, 1) ;

    //first thing in send is hash of the message
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aContentHash.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    // after hash, the size of content:
    quint32 lengthNetworkByteorder ( htonl( (quint32)(aContent.size()) ) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    // followed by content:
    retval.append(aContent) ;
    lengthNetworkByteorder = htonl( (quint32)(aSignature.size()) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    retval.append(aSignature) ;

    quint32 timestampNetworkBO ( htonl( aTimeStamp ) ) ;
#ifndef WIN32
#ifdef DEBUG
    time_t contentTime_T ( aTimeStamp ) ;
    char timebuf[40] ;
    LOG_STR2("Private message time of publish at send: %s", ctime_r(&contentTime_T,timebuf)) ;
#endif
#endif
    retval.append((const char *)&timestampNetworkBO, sizeof(quint32))  ;

    // then destination node:
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aDestination.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    // then recipient:
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aRecipient.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::replyToAdsClassified(const Hash& aClassificationHash,
        const QList<QPair<Hash,quint32> >& aListOfAds	) {
    QByteArray retval ;
    retval.append((const char *)&KListOfAdsClassifiedAtHash, sizeof(unsigned char)) ;

    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aClassificationHash.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }
    quint32 numberArticles ( htonl( aListOfAds.size() ) ) ;
    retval.append((const char *)(&numberArticles), sizeof(quint32)) ;
    for ( int k = 0 ; k < aListOfAds.size() ; k++ ) {
        for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
            quint32 hashBitsNetworkBO ( htonl( aListOfAds[k].first.iHash160bits[i] ) ) ;
            retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
        }
        quint32 ts (  htonl( aListOfAds[k].second ) ) ;
        retval.append((const char *)(&ts), sizeof(quint32)) ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::searchSend(const QString& aSearch,
        bool aSearchAds,
        bool aSearchProfiles,
        bool aSearchComments,
        const Hash& aSearchIdentifier) {
    QByteArray retval ;
    retval.append((const char *)&KSearchRequest, sizeof(unsigned char)) ;
    quint32 searchflags ( 0x0 ) ;
    if ( aSearchAds ) {
        searchflags = searchflags | 0x01 ;
    }
    if ( aSearchProfiles ) {
        searchflags = searchflags | 0x01 << 1 ;
    }
    if ( aSearchComments ) {
        searchflags = searchflags | 0x01 << 2 ;
    }
    quint32 flagBitsNetworkBO ( htonl( searchflags ) ) ;
    retval.append((const char *)(&flagBitsNetworkBO), sizeof(quint32)) ;

    quint32 searchIdNetworkBO ( htonl( aSearchIdentifier.iHash160bits[0] ) );
    retval.append((const char *)(&searchIdNetworkBO), sizeof(quint32)) ;

    QByteArray searchStringUtf8 ;
    searchStringUtf8.append(reinterpret_cast<const char *>(aSearch.toUtf8().constData())) ;
    if ( searchStringUtf8.size() > 20*1024 ) {
        // this is ridiculous
        searchStringUtf8 = QString("bananas").toUtf8() ;
    }
    quint32 sizeNetworkBO ( htonl(searchStringUtf8.size()) ) ;
    retval.append((const char *)(&sizeNetworkBO), sizeof(quint32)) ;
    retval.append(searchStringUtf8) ;
    return retval ;
}

QByteArray ProtocolMessageFormatter::searchResultsSend(const QList<SearchModel::SearchResultItem>& aResults,
        quint32 aSearchId) {
    // search results are actually plain json text.
    // there is magic, length of the json and then
    // the actual json.
    QByteArray retval ;
    retval.append((const char *)&KSearchResults, sizeof(unsigned char)) ;
    const QVariant toplevel
    ( SearchModel::serializeSearchResults(aResults,
                                          aSearchId) ) ;
    const QByteArray resultBytes ( JSonWrapper::serialize(toplevel) ) ;
    if ( resultBytes.size() > 0 ) {
        quint32 sizeNetworkBO ( htonl(resultBytes.size()) ) ;
        retval.append((const char *)(&sizeNetworkBO), sizeof(quint32)) ;
        retval.append(resultBytes) ;
    } else {
        retval.clear() ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::voiceCall(const VoiceCall& aCall,
        MController& aController,
        const Hash& aSelectedProfile,
        bool aDoSign ) {
    QByteArray retval ;
    if ( aCall.iOkToProceed ) {
        retval.append((const char *)&KVoiceCallStart, sizeof(unsigned char)) ;
    } else {
        retval.append((const char *)&KVoiceCallEnd, sizeof(unsigned char)) ;
    }
    const QByteArray resultBytes ( aCall.asJSon() ) ;
    if ( resultBytes.size() > 0 ) {
        quint32 sizeNetworkBO ( htonl(resultBytes.size()) ) ;
        retval.append((const char *)(&sizeNetworkBO), sizeof(quint32)) ;
        retval.append(resultBytes) ;
        QByteArray signature ;
        if ( aDoSign ) {
            if ( aController.model().contentEncryptionModel().sign(
                        aSelectedProfile,
                        resultBytes,
                        signature ) == 0 ) {
                // 0 is success
                quint32 signatureSizeNetworkBO ( htonl(signature.size()) ) ;
                retval.append((const char *)(&signatureSizeNetworkBO), sizeof(quint32)) ;
                retval.append(signature) ;
            } else {
                QLOG_STR("ERROR: Voice call signing failed") ;
                retval.clear() ;
            }
        } else {
            // put in a zero-size signature
            quint32 signatureSizeNetworkBO ( 0 ) ;
            retval.append((const char *)(&signatureSizeNetworkBO), sizeof(quint32)) ;
        }
    } else {
        retval.clear() ;
    }
    return retval ;
}

QByteArray ProtocolMessageFormatter::voiceCallRtData(quint32 aCallId,
                                                     quint32 aSeqNo,
                                                     VoiceCallEngine::PayloadType aPayloadType,
                                                     const QByteArray& aPayload ) {
    QByteArray retval ;

    switch ( aPayloadType ) {
    case VoiceCallEngine::Audio: {
        retval.append((const char *)&KRealtimeData, sizeof(unsigned char)) ;
        retval.append((const char *)&KRTDataAudioSubtype, sizeof(unsigned char)) ;
        quint32 callIdNetworkBO ( htonl(aCallId ) ) ; 
        retval.append((const char *)(&callIdNetworkBO), sizeof(quint32)) ; 
        quint32 seqNoNetworkBO ( htonl(aSeqNo ) ) ; 
        retval.append((const char *)(&seqNoNetworkBO), sizeof(quint32)) ; 
        quint32 payloadSizeNetworkBO ( htonl(aPayload.size() ) ) ; 
        retval.append((const char *)(&payloadSizeNetworkBO), sizeof(quint32)) ;
        retval.append(aPayload) ; 
        }
        break ; 
    default:
        // do not append anything, zero-size requests are not processed
        break ; 
    }
    return retval ; 
}

QByteArray ProtocolMessageFormatter::profileCommentPublish(const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const Hash& aProfileCommented,
        quint32 aTimeStamp,
        quint32 aFlags ) {
    return doCommentSendOrPublish(aContentHash,
                                  aContent,
                                  aSignature,
                                  aBangPath,
                                  aProfileCommented,
                                  aTimeStamp,
                                  aFlags,
                                  true );
}

QByteArray ProtocolMessageFormatter::profileCommentSend(const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const Hash& aProfileCommented,
        quint32 aTimeStamp,
        quint32 aFlags ) {
    QList<quint32>  dummyBangPath;
    return doCommentSendOrPublish(aContentHash,
                                  aContent,
                                  aSignature,
                                  dummyBangPath,
                                  aProfileCommented,
                                  aTimeStamp,
                                  aFlags,
                                  false );

}

QByteArray ProtocolMessageFormatter::doCommentSendOrPublish(const Hash& aContentHash,
        const QByteArray& aContent,
        const QByteArray& aSignature,
        const QList<quint32>& aBangPath,
        const Hash& aProfileCommented,
        quint32 aTimeStamp,
        quint32 aFlags,
        bool aIsPublish) {
    // ok, lets decide here the layout of comment publish..
    // first magick number telling what it is.
    // then bangpath 5 items and if not all 5 are filled,
    // the remaining ones are filled with zeroes.

    // then content hash, 5*32 bytes
    // then commented profile hash, 5*32 bytes

    // then length of the actual data, followed by the actual data
    // then length of the signature, followeds by the signature.
    // then flags, one unsigned 32-bit.
    //  * if bit 1 is up, it means that content is encrypted.
    // then time when content was published.


    unsigned char ch ( aIsPublish ? KProfileCommentPublish : KProfileCommentSend ) ;
    QByteArray retval ;
    retval.append((const char *)&ch, 1) ;

    if ( aIsPublish ) { // if in send-mode, just omit bangpath
        quint32 bPathNumberNetworkByteorder ;

        if ( aBangPath.size() >= 1 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[0]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 2 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[1]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 3 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[2]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 4 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[3]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
        if ( aBangPath.size() >= 5 ) {
            bPathNumberNetworkByteorder = htonl(aBangPath[4]);
        } else {
            bPathNumberNetworkByteorder = 0 ;
        }
        retval.append((const char *)(&bPathNumberNetworkByteorder), sizeof(quint32)) ;
    }
    // now has 5 numbers as bangpath, then put hash of content
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aContentHash.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }

    // now has 5 numbers as bangpath+content hash, then fingerprint of commented profile
    for ( int i = 0 ; i < Hash::KNumberOfIntsInHash ; i++ ) {
        quint32 hashBitsNetworkBO ( htonl( aProfileCommented.iHash160bits[i] ) ) ;
        retval.append((const char *)(&hashBitsNetworkBO), sizeof(quint32)) ;
    }

    // now has bangpath+content hash+profile hash, follow the content+signature:
    // after hash, the size of content:
    quint32 lengthNetworkByteorder ( htonl( (quint32)(aContent.size()) ) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    // followed by content:
    retval.append(aContent) ;
    lengthNetworkByteorder = htonl( (quint32)(aSignature.size()) ) ;
    retval.append((const char *)(&lengthNetworkByteorder), sizeof(quint32)) ;
    retval.append(aSignature) ;
    // then flags:
    quint32 flagsNetworkByteOrder ( htonl(aFlags) ) ;
    retval.append((const char *)(&flagsNetworkByteOrder), sizeof(quint32)) ;
    // and finally timestamp of publish
    quint32 timestampNetworkBO ( htonl( aTimeStamp ) ) ;
    retval.append((const char *)&timestampNetworkBO, sizeof(quint32))  ;
    return retval ;
}


