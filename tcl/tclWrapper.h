/*     -*-C++-*- -*-coding: utf-8-unix;-*-
  Classified Ads is Copyright (c) Antti Järvinen 2013-2017.

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


#ifndef TCLWRAPPER_H
#define TCLWRAPPER_H
#include <QThread>
#include <QQueue>
#include "../mcontroller.h"
#include <tcl.h>

class Model ;
class MController ;
class TclConsoleDialog ;
class TclCallbacks ;

/**
 * @brief Class providing TCL interpreter services
 *
 * This class wraps [TCL](http://www.tcl.tk) interpreter in 
 * [safe mode](https://www.tcl.tk/man/tcl8.6/TclCmd/safe.htm). 
 * This class is intended to be used so that, this 
 * is instantiated once and deleted at application shutdown. 
 * One TCL program may be running at same time - restriction of 
 * Tk is that one instance per process only.
 *
 * This is a thread and it is supposed to be used so that
 * class is instantiated, setScript() method is called,
 * followed by start(). Start may be called multiple times,
 * calling start() when thread is already executing has
 * no effect. In user interface user may open the TCL
 * console dialog from menu, commands typed are evaluated
 * using an interpreter. In addition to "ad-hoc" commands
 * to interpreter, a program text may be stored into 
 * TCL program library. 
 *
 * Classified-ads implements a local data storage for TCL
 * programs but a precondition for its successful usage is 
 * that program being run has been previously stored into
 * TCL program library of classified ads. Reason for this 
 * the underlying database implementation: each program may
 * have its own local data but for being able to associate 
 * the program with the data, the program must be stored
 * first. Longer TCL program may be typed directly into
 * "TCL Programs" dialog but for really long programs it
 * is naturally easier to use a proper code-editor and then
 * just copy-paste the code in classified-ads as it does not
 * implement a proper code-editor, just a "normal" text editor.
 * TCL interpreter output for debugging purposes may be seen
 * at "TCL Interpreter" dialog in the UI. Stdout+stderr of
 * the interpreter are directed there. At time of writing there
 * is known issue with very long output lines and output lines
 * containing non-ASCII7 -characters like accented characters
 * of finnish language. Using those will not stop the interpreter
 * or program but will break interpreter output. 
 *
 * In addition to standard safe-mode-TCL/TK this wrapper
 * introduces several TCL extension commands for accessing
 * features implemented uniquely by classified-ads software. 
 * These features include for example search, fetch and post
 * of an classified ad. List of additional commands is
 *
 *   TCL command                              | Purpose
 *   ---------------------------------------  | -------------
 *   [listProfiles](@ref listProfilesLabel)   | Returns list of profile id's matching given conditions
 *   [listComments](@ref listCommentsLabel)   | Returns list of profile comment id's matching given conditions
 *   [listAds](@ref listAdsLabel)             | Returns list of classified ad id's matching given conditions
 *   [getProfile](@ref getProfileLabel)       | Returns profile details by id
 *   [getProfileComment](@ref getCommentLabel)| Returns profile comment by id
 *   [getClassifiedAd](@ref getAdLabel)       | Returns classified ad by id
 *   [getBinaryFile](@ref getFileLabel)       | Returns binary file by id
 *   [getDbRecord](@ref getDbRecordLabel)     | Returns database records by query
 *   [publishFile](@ref pFileLabel)           | Publishes a binary file to DHT for other operators to retrieve
 *   [publishProfile](@ref pPLabel)           | Publishes operators profile
 *   [publishProfileComment](@ref pPCLabel)   | Publishes a new comment regarding some operator
 *   [publishClassifiedAd](@ref pAdLabel)     | Publishes a new classified ad
 *   [publishDbRecord](@ref pDbRecordLabel)   | Publishes a new or updated database record
 *   [calculateSHA1](@ref calcSHA1Label)      | Command for calculating SHA1 hash
 *   [storeLocalData](@ref storeDLabel)       | Stores data into local storage in operators computer
 *   [retrieveLocalData](@ref retrieveDLabel) | Retrieves TCL-program specific data prevously saved
 *   [openFileSystemFile](@ref openFSFLabel)  | Opens file from local file system
 *   [saveFileSystemFile](@ref saveSFFLabel)  | Saves file to local file system
 *   [isProfileTrusted](@ref isPTrustedLabel) | Performs look-up to operator trust-tree
 *
 * Longer documentation of each command follows. 
 * 
 * listProfiles {#listProfilesLabel}
 * ============
 * Basic usage of listProfiles is 
 * ```
 * set profileList [ listProfiles foobar ]
 * ```
 * after which profileList contans a (possibly empty) array of
 * profile idenfiers. The search condition here is the word
 * "foobar" that is used to match profiles from search index. 
 * Returned list is list of dictionaries. Each dictionary
 * has as its key profile address (SHA1) and in its value the
 * "display name" of the profile, that is human-readable combination
 * of profile address, nickname, given and family names. 
 *
 * The search engine in use is the powerful full-text-search (FTS) 
 * feature of the sqlite database. It is well documented in 
 * [SQLite documentation](https://www.sqlite.org/fts3.html). 
 * For a profile to be indexed for search purposes, it must be
 * a public profile. The fields that are put into index are
 *  * profile address (SHA1 identifier), field name = profilehash
 *  * nickname, field name = nickname
 *  * greeting text, field name = greetingtext
 *  * first name, field name = firstname
 *  * family name, field name = familyname
 *  * city, field name = city
 *  * btc wallet address, field name = btc
 *  * state, field name = state
 *
 * Example queries include
 * ```
 * set profileList [ listProfiles {a*} ]
 * ```
 * that set value of `profileList` to contain every profile from local
 * storage that in some indexed field contains a word that begins 
 * with letter "a". 
 * ```
 * set profileList [ listProfiles {Suomi Finland} ]
 * ```
 * that returns every profile that in some indexed field
 * contain word "Suomi" and also word "Finland". If only 
 * "Suomi" or "Finland" is present, profile is not returned
 * ```
 * set profileList [ listProfiles {Suomi OR Finland} ]
 * ```
 * that returns every profile that in some indexed field
 * contain word "Suomi" or word "Finland".
 * ```
 * set profileList [ listProfiles {nickname:s*} ]
 * ```
 * Returns profiles that in nickname field have a word that
 * begins with letter "s". Example content of $profileList from such a 
 * query might be for example
 * 
 * ```
 * {1D2CB451BD348B12BA39316499F5AF6D53ACEECF Simo} 
 * {22C13DCF54C8D9E6148C834E601683DD894FCBF5 {Elmeri Sukeltaja}} 
 * {52F79319C034BA9939E930A5BD812B6CBE5DF2DD {Susanna K}}
 * ```
 *
 * Search is performed on local data only
 * 
 * listComments {#listCommentsLabel}
 * ============
 * listComments is similar in function as command
 * [listProfiles](@ref listProfilesLabel) but it returns profile 
 * comments instead.
 * ```
 * set commentList [ listComments foobar ]
 * ```
 * Would return every profile comment with word foobar in text. 
 * Indexed fields are
 *  * Commented profile identifier (SHA1), field name = profilehash
 *  * Author profile identifier (SHA1), field name = commentorhash
 *  * Comment text , field name = comment
 *  * Comment subject line, field name = commentsubject
 *  * Nickname of autoher, field name = commentornickname
 *
 * Results are returned in same manner, as a list of dictionaries
 * where key is the comment SHA1 identifier and value is
 * comment subject. 
 *
 * listAds {#listAdsLabel}
 * ============
 * listAds is similar in function as command
 * [listProfiles](@ref listProfilesLabel) but it returns identifiers
 * of classified ads instead.
 * ```
 * set adList [ listAds computer ]
 * ```
 * Would return every classified ad with word computer in any indexed field. 
 * Indexed fields are
 *  * Author profile identifier (SHA1), field name = senderhash
 *  * Author profile display name, field name = sendername
 *  * Subject, field name = subject
 *  * Text of the ad, field name = text
 *
 * Results are returned in same manner, as a list of dictionaries
 * where key is the SHA1 identifier of the classified ad and value is
 * ad subject. 
 *
 * getProfile {#getProfileLabel}
 * ============
 * getProfile returns details of operator profile. Argument is the 
 * SHA1 identifier of the profile. Profile is returned as a dictionary
 * where keys are mapped to their values. 
 * ```
 * set p [ getProfile $profileIdentifier ]
 * ```
 * The keys in the dictionary are the following
 * 
 *  
 *   Key                | Description of the value
 *   ------------------ | ----------------------------------
 *   displayName        | Human-readable name of profile
 *   fingerPrint        | Profile SHA1 address
 *   greetingText       | Operator greeting text
 *   firstName          | Operator given name
 *   familyName         | Operator family name
 *   cityCountry        | Operator city/country
 *   timeOfPublish      | Time of last publish, as seconds since 1-Jan-1970
 *   isPrivate          | Boolean indicating if profile is private
 *   imagePNG           | Image data of profile picture
 *   BTCAddress         | BTC wallet address of the operator
 *   stateOfTheWorld    | Situation with the world, as perceived by the operator
 *   profileReaders     | List of profiles readers, value is list of operator profiles
 *   sharedFiles        | List of files shared by profile, value is list of file SHA1 identifiers
 *   trustList          | List of profiles publicly trusted by this operator
 *
 * Every field may be omitted from dictionary if there is no value. 
 * Value is always present in fields fingerPrint, isPrivate, timeOfPublish 
 * and displayName. Note that operators own profile may not have been 
 * published at all and for this reason getting operators own profile 
 * may fail. Inside TCL programs the current profile SHA1 is stored in 
 * global variable `::profileInUse` so command `getProfile $::profileInUse` 
 * should always return details of operators own profile, if published. 
 *
 * getProfileComment {#getCommentLabel}
 * ============
 * getProfileComment returns single comment to some operators profile. 
 * Argument is the  SHA1 identifier of the comment as returned by
 * [listComments](@ref listCommentsLabel) command. Comment is returned 
 * as a dictionary where keys are mapped to their values. 
 * ```
 * set c [ getProfileComment $commentIdentifier ]
 * ```
 * The keys in the dictionary are the following
 *  
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   fingerPrint            | Comment SHA1 identifier
 *   fingerPrintOfCommented | SHA1 identifier of the operator that comment concerns
 *   fingerPrintOfSender    | SHA1 identifier of the author of the comment
 *   timeOfPublish          | Time of comment publish, as seconds since 1-Jan-1970
 *   isPrivate              | Boolean indicating if comment is private
 *   senderName             | Display name of profile of author of comment
 *   subject                | Comment subject
 *   messageText            | Comment text in html format
 *   plainMessageText       | Comment text in plain text
 *   replyTo                | SHA1 id of comment that this comments is a reply to
 *   attachedFiles          | List of files attached to this comment, value is list of file SHA1 identifiers
 *
 * getClassifiedAd {#getAdLabel}
 * ============
 * getClassifiedAd is similar in function to [getProfileComment](@ref getCommentLabel) 
 * command but it returns an classified ad. 
 * ```
 * set a [ getClassifiedAd $SHA1Identifier ]
 * ```
 * The keys in returned dictionary are the following
 *  
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   fingerPrint            | Ad SHA1 identifier
 *   displayName            | Human-readable subject of the ad
 *   timeOfPublish          | Time of comment publish, as seconds since 1-Jan-1970
 *   senderName             | Display name of profile of author of ad
 *   senderHash             | SHA1 identifier of author of the ad
 *   subject                | Comment subject
 *   messageText            | Comment text in html format
 *   plainMessageText       | Comment text in plain text
 *   replyTo                | SHA1 id of ad that this ad is a reply to
 *   attachedFiles          | List of files attached to this ad, value is list of file SHA1 identifiers
 *   aboutComboboxText      | In "ads" view, value of left-side classification selection combobox
 *   concernsComboboxText   | In "ads" view, value of middle classification selection combobox
 *   inComboboxText         | In "ads" view, value of right-side classification selection combobox
 *   aboutComboBoxIndex     | In "ads" view, selection index of left-side classification selection combobox
 *   inComboBoxIndex        | In "ads" view, selection index of middle classification selection combobox
 *   concernsComboBoxIndex  | In "ads" view, selection index of right-side classification selection combobox
 *   group                  | Combination of "concerning.where",f.ex. "ToBeSold.Cars.UnitedStates"
 * 
 *
 * getBinaryFile {#getFileLabel}
 * ============
 * getBinaryFile is similar in function to [getProfileComment](@ref getCommentLabel) 
 * command but it returns a binary file. 
 * ```
 * set f [ getBinaryFile $fileSHA1 ]
 * ```
 * The keys in returned dictionary are the following
 *  
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   fingerPrint            | Ad SHA1 identifier
 *   timeOfPublish          | Time of comment publish, as seconds since 1-Jan-1970
 *   mimeType               | Mime-type of the file, like 'application/tcl'
 *   description            | Description of the content
 *   owner                  | SHA1 of profile who published the file
 *   contentOwner           | Name (string) of possible (copyright) owner of content
 *   license                | License under which the file may be further shared
 *   fileData               | Actual binary data of the file
 *
 * getDbRecord {#getDbRecordLabel}
 * ============
 * Classified ads has concept of shared database. It is a general-purpose
 * database where database records are shared between nodes in the 
 * network. Queries to data may be made get fetch records, queries 
 * may also be sent to neighboring nodes in the network to check
 * there might be any database records that would fit the query. 
 * In C++ api fetching and retrieving of records happens via 
 * @ref CaDbRecordModel and database-related network operations are
 * implemented in class @ref DbRecordRetrievalEngine. This getter
 * method makes a query to local database and also sends the same
 * query to neighboring nodes. 
 *
 * Shared database has concept of "collection" and database records
 * in same collection are supposed to somehow logically belong 
 * together. For example product items available from a shop might
 * go to one collection, purchase orders of same items might belong
 * to anohter collection. In classified ads collection is just
 * SHA1 hash that may be calculated from string data. 
 *
 * Actual data content is binary data and it can not be queried. Classified
 * ads makes no assumptions about data content and therefore does not 
 * try to even index the data. When [publishDbRecord](@ref pDbRecordLabel)
 * is called, programmer must set database record metadata so, that
 * record may be later successfully found. At simplest form a word
 * describing the content is included in metadata - for example if
 * there is pet-shop operating inside classified ads, it might have
 * animals for sale and rent and while publishing the product listing
 * the indexed metadata of records might contain word "cow" for each
 * db recort that in binary data blob contains details about particular
 * cow for sale or rent - end-user may then perform a query regarding
 * cows in this particular product collection and end up with 0 or more
 * bovines.
 *
 * Queries may be done in several ways but basic form is
 *
 * ```
 * dict set criteria collectionId [ calculateSHA1 {Pet-shop products} ]
 * dict set criteria searchPhrase cow
 * set recordsFound [ getDbRecord $criteria ]
 * ```
 * That would return all records from collection "Pet-shop products"
 * and that contain word "cow" in search text. The criteria is a 
 * tcl dictionary, the database records are returned as a list of
 * dictionaries. Content of dictionaries is detailed below. 
 *
 *   Search dictionary key      | Value
 *   -------------------------  | -------------
 *   recordId                   | Simplest query. If record SHA1 id is known, this fetches the record
 *   collectionId               | Specifies collection SHA1. Only mandatory value.
 *   senderId                   | Operator address SHA1. Include only records from this operator
 *   searchPhrase               | Searchphrase in [SQLite FTS](https://www.sqlite.org/fts3.html) format
 *   searchNumber               | Returns records having exactly this indexed integer
 *   searchNumberLessThan       | Returns records where search number is smaller than given number
 *   searchNumberMoreThan       | Returns records where search number is greater than given number
 *
 * Idea behind searchPhrase and searchNumber is that when record is
 * stored (published) programmer may assign a text and a number to the
 * record. Phrase is just text, it may contain several words and it
 * is indexed with Sqlite FTS method. Records may then be queried
 * using the FTS that goes along the database record database table. 
 * Example usage would go like this:
 * ```
 * dict set criteria collectionId [ calculateSHA1 {Pet-shop products} ]
 * dict set criteria searchPhrase {cow priceCategory3*} ]
 * dict set criteria searchNumberLessThan 15 ]
 * set recordsFound [ getDbRecord $criteria ]
 * ```
 * All conditions in the criteria must be met for each record to be included.
 * In above example the pet-shop products are queried regarding records
 * that previously have been assigned a search number with value less than
 * 15 and published with a search phrase that contains both words "cow"
 * and another word that begins with "priceCategory3". 
 *
 * Records are returned in TCL list containing dictionaries. Each dictionary
 * presents one record in the database. Keys in the returned dictionary
 * are the following
 *   Db record dictionary key   | Value
 *   -------------------------  | -------------
 *   recordId                   | Record identifier. Used together with senderId uniquely identifies the record
 *   collectionId               | Specifies collection SHA1.
 *   senderId                   | SHA1 address of operator who published the record
 *   searchPhrase               | Original search phrase
 *   searchNumber               | Original search number
 *   data                       | Actual payload of the record - any binary data
 *   isSignatureVerified        | Boolean indicating record RSA signature was checked agaist key of senderId
 *   recordRecipients           | List of operator SHA1 addresses to whom the record was encrypted to
 *   encrypted                  | Boolean indicating if record was published encrypted
 *   timeOfPublish              | Time when record was published as seconds since 01-Jan-1970
 *
 * As the API to the shared database here is implemented as set of
 * TCL commands, it is easiest to carry the actual payload (member "data" in 
 * dictionary) in some TCL data structure like list or dictionary but
 * that is naturally up to programmer to decide. In underlying storage
 * the database data is stored in zlib-compressed format so trying to
 * compress the content prior to publish usually makes situation worse. After
 * compression the data must fit limits set by DHT implementation of
 * classified ads, meaning maximum size slightly less than 2 megabytes
 * per record. For application processing larger amounts of data the
 * data must be split into smaller records.
 *
 * It is also worth noting that database content can really be anything.
 * Database records can originate from any operator, regardless of collection.
 * Implementation can not guarantee any format regarding actual data
 * in the records. Bugs in TCL program implementations will result
 * in faulty data records being published so extreme care is in order
 * while parsing the database record contents. There are numerous articles
 * concerning TCL and untrusted input data ; special care must be taken
 * to prevent calls to TCL functions that would evaluate the content as
 * a TCL script, see [TCL wiki](http://wiki.tcl.tk/9749) for examples.  
 *
 * publishFile {#pFileLabel}
 * ============
 * This command is used to publish a binary file to DHT for other
 * operators to retrieve. Unlike [database records](@ref pDbRecordLabel)
 * there is no search function over binary files ; the file SHA1
 * must be known for its retrieval later. Most often published files 
 * are used as attachment to classified ads, profile comments and included
 * in list of operators shared files. 
 *
 * Basic example of file publish is
 * ```
 * dict set f fileName README.TXT
 * dict set f description {This is a very fine text file}
 * dict set f license {Creative commons CC0}
 * dict set f fileData {Actual content of the file is here, can be anything}
 * dict set f forceNoEncryption true
 * set fileHash [ publishFile $f ]
 * puts [ format {Fingerprint of README.TXT is %s} $fileHash ]
 * ```
 * In addition to dictionary keys listed in above example it is possible
 * add to dictionary a list with dictionary key `fileRecipients` whose
 * value is a TCL list containing SHA1 addresses of operators whose
 * public RSA keys will be used to encrypt the file content. It is up
 * to programmer to ensure that RSA keys of named operators are present
 * in local data storage before the operation. If operators profile can
 * be retrieved using [getProfile](@ref getProfileLabel) then encrypting
 * content to be readable by that operator is likely to succeed. 
 *
 * Normally file is published using operators profile settings: if operator
 * has private profile, files are published so that content is encrypted to
 * be readable only by those operators that are listed as readers of 
 * publishing operators profile. Dictionary key `forceNoEncryption` skips 
 * encryption always, making the file readable by all. Using 
 * `forceNoEncryption` together with `fileRecipients` is silly.
 *
 * publishProfile {#pPLabel}
 * ============
 * Command `publishProfile` publishes operators own profile for others
 * to see. It is equivalent to pressing button "publish" in "own profile"
 * page of user interface. 
 *
 * This command is present in classified ads since v0.13. 
 *
 * publishProfileComment {#pPCLabel}
 * ============
 * Command `publishProfileComment` is equivalent to pressing the "Comment"
 * button on profile display UI, typing and sending the comment. 
 * Example usage might go along following example:
 * ```
 * # find profile fingerprint of Erkkielvis
 * set profiles [ listProfiles {nickname:Erkkielvis} ]
 * set profileFP [ dict keys [ lindex $profiles 0 ] ]
 * # construct a comment
 * set d [ dict create fingerPrintOfCommented $profileFP ]
 * dict set d senderName {Eino Reino Leino} 
 * dict set d subject {Message subject goes here} 
 * dict set d messageText {Lenghty message text and Greetings from outer space.} ]
 * # and send it away
 * set hash [ publishProfileComment $d ]
 * puts [ format {Fingerprint of new comment is %s} $hash ]
 * ```
 * In addition to dictionary keys presented in above example, following keys
 * may be also present in the dictionary during publish. 
 *   Key                    | Description of the value
 *   ---------------------- | ----------------------------------
 *   replyTo                | SHA1 id of comment that this comment is a reply to
 *   attachedFiles          | List of files attached to this comment, value is list of file SHA1 identifiers
 *
 * In practice the attachment files must have been previously published 
 * using [publishFile](@ref pFileLabel) command.
 *
 * publishClassifiedAd {#pAdLabel}
 * ============
 * Command is used to send a new classified ads for whole world to see.
 * Usage is similar to [publishProfileComment](@ref pPCLabel) command as
 * the ad is first constructed as a TCL dictionary and then
 * given to publish command. SHA1 fingerprint of the new ad item
 * is returned. 
 *
 * When replying to existing ad, it is important to exactly copy 
 * the classification-related fields from replied article because
 * they affect the classification of the ad and user interface in
 * turn exclusively uses classification when searching for articles. 
 * If classification does not match, reply very easily goes un-noticed
 * by readers of the original message chain. 
 *
 * ```
 * dict set new_ad senderName {Eino-Reino the cosmonaut}
 * dict set new_ad subject {Orbiting the moon is for extraordinary cows}
 * dict set new_ad plainMessageText {TCL is fully usable in outer space.}
 * dict set new_ad aboutComboboxText {Testing}
 * dict set new_ad concernsComboboxText {Classifed ads software}
 * dict set new_ad inComboboxText {Moon orbit}
 * set ad_hash [ publishClassifiedAd $new_ad ]
 * puts [ format {Fingerprint of new ad is %s} $ad_hash ]
 * ```
 *
 * publishDbRecord {#pDbRecordLabel}
 * ============
 * Before database record can be retrieved using 
 * [getDbRecord](@ref getDbRecordLabel) it must be published in some 
 * node in the network. DHT takes care of storing the database record
 * in correct nodes so that they'll be later found as queried. 
 * In practice the collection of the record is used to determine 
 * the network nodes that will store the record. 
 *
 * Same principle that holds true all items that may be posted to
 * internet holds true here too: once published, it is impossible to
 * permanently remove the once-published content. In classified-ads 
 * it is possible to replace database record with new content, also
 * with empty content but due to very nature of peer-to-peer 
 * distributed hash table it is possible that someone somewhere still
 * has the old or original record. 
 *
 * Classified ads distributed database has no concept of "commit" like
 * some relational databases. A database record is published, it is copied
 * to some nodes in the network and it may be queried. It is possible that
 * upon query not all records that since beginning of time have been
 * published are always returned in query if they should match, or that 
 * the very latest record is returned. Keeping this philosophy in mind, 
 * that classified ads distributed database is not accurate in query 
 * results in same sense as databases that are based on client-server 
 * principle, the applications need to be designed around this fact. 
 * Old records that have not been queried for a very long time are simply
 * forgotten. On the other hand a DHT should be able to handle vast amounts
 * of data as each node in the network adds a little bit additional
 * storage and processing capacity to the whole network. 
 *
 * Semantics of `searchPhrase` and `searchNumber` are already discussed in
 * [getDbRecord](@ref getDbRecordLabel) and usually it is very useful to
 * set one or both of them to some value that describes the content. 
 *
 * Basic usage of publishDbRecord is 
 * ```
 * dict set recordToSave collectionId [ calculateSHA1 {Name of db collection} ]
 * dict set recordToSave searchPhrase {Cow cows bovines}
 * dict set recordToSave searchNumber 42
 * dict set recordToSave encrypted false
 * dict set recordToSave data {This is actual record content, usually a TCL data structure}
 * publishDbRecord $recordToSave
 * ```
 * The record that got published in example, can later be retrieved using
 * the collection id together with search number and/or phrase. If db record is
 * published encrypted, the readers of the current operator profile
 * are the readers of the record. That may be overridden using key
 * `recordRecipients` in dictionary, its value is list of operator SHA1 
 * identifiers whose public RSA keys must be present in local storage
 * and the db record will be made readable to those operators. 
 *
 * calculateSHA1 {#calcSHA1Label}
 * ============
 * Calculates SHA1 digest over data
 * ```
 * set SHA1sum [ calculateSHA1 {Cow belongs to low-flying bovines} ]
 * ```
 *
 * storeLocalData {#storeDLabel}
 * ============
 * Command for storing local data works in about same manner as 
 * [publishDbRecord](@ref pDbRecordLabel) with exception that this
 * data is not sent to any other node of the network. Local data
 * may only be retrieved from the same program that was used
 * to store it. Classified ads keeps local data in same db
 * table with the TCL program data and if program is deleted, its
 * local data is deleted too. If TCL program is modified and
 * saved again in "TCL Programs" dialog, it is still considered
 * same program e.g. it will find local data also after modifications.
 * If name of the program changes when saving the changes to TCL program,
 * the new program with new name is considered a new program and
 * its local data block is initially empty. 
 *
 * ```
 * set x {any binary data}
 * storeLocalData $x
 * ```
 *
 * retrieveLocalData {#retrieveDLabel}
 * ============
 * Command may be used to bring back data previously stored 
 * using [storeLocalData](@ref storeDLabel) like in this example
 * ```
 * set x [ retrieveLocalData foobar ]
 * puts [ format {len of local data is %d} [ string length $x ] ]
 * ```
 * For local storage to work in meaningful way the TCL program 
 * being run must be saved using "TCL Programs" dialog as local
 * data is kept in same db table with the programs that access them. 
 *
 * openFileSystemFile {#openFSFLabel}
 * ============
 * Command may be used to ask user to select a file system file
 * whose content will be returned as return value 
 * ```
 * set x [ openFileSystemFile ]
 * ```
 * command may be given additional additional specifying file pattern,
 * like `openFileSystemFile *.png` would offer for open only files
 * whose name end with `.png`.
 *
 * saveFileSystemFile {#saveSFFLabel}
 * ============
 * Command may be used to save data into file system file. 
 * ```
 * set fileContent {A cow filet in a file}
 * saveFileSystemFile $fileContent *.txt
 * ```
 * Would ask user which .txt -ending file name shall be used
 * to store content of variable `fileContent`.
 *
 * isProfileTrusted {#isPTrustedLabel}
 * ============
 * Does a look-up in @ref TrustTreeModel to find out if given operator
 * specified by SHA1 address is found from trust tree or not:
 * ```
 * # find profile fingerprint of Maud
 * set profiles [ listProfiles {nickname:Maud} ]
 * set profileFP [ dict keys [ lindex $profiles 0 ] ]
 * # and do a query
 * puts [ format {Is Maud trusted %s} [ isProfileTrusted $profileFP ] ]
 * ```
 * Trust-tree is populated from operators user interface where contacts
 * may be assigned a "public trust" value that is then used to construct
 * the trust tree. Operators trusted by operators in your trust-list
 * are considered trusted too. 
 *
 */
class TclWrapper : public QThread {
    Q_OBJECT
public:
    TclWrapper(Model& aModel,
               MController& aController);

    /**
     * Method for tearing down the interpreter
     */
    ~TclWrapper() ;
    /**
     * stop possible running script.
     *
     * @aDeleteLater if set to true, will cause the instance to
     * delete itself after interpreter is deleted
     */
    void stopScript(bool aDeleteLater = false ) ;
    void setScript(const QString& aScript) ; /**< TCL to evaluate, called before start */
    /**
     * Returns the script currently being evaluated
     */
    const QString& currentProgram() const { return iTCLScript ; } ;
    void showConsole() ; /**< Displays TCL interpreter console dialog */
    /**
     * method for receiving notifications about data item
     * additions to data model. this is called for example when
     * new classified as are added. See also
     * @ref notifyInterpreterOfContentReceived.
     */
    void notifyOfContentReceived(const Hash& aHashOfContent,
                                 const ProtocolItemType aTypeOfReceivedContent ) ;
public slots:
    void run() ;

signals:
    /**
     * this is not method but signal ; if in error, get emit()ted
     */
    void error(MController::CAErrorSituation aError,
               const QString& aExplanation) ;
    /**
     * This slot may be connected to display of TCL interpreter
     * textual output
     */
    void consoleOutput(QString aOutput) ;
public slots:
    void consoleClosed() ;
    /**
     * this slot may be used to inject more TCL to be
     * interpreted
     * @param aScript is the TCL script to be evaluated
     * @param aMainWindowTitle contains optional main window title.
     *        This is used only in case interpreter is not yet
     *        running and this may then be used to set the title
     *        of the tk main window during interpreter initialization.
     */
    void evalScript(QString aScript,
                    QString* aMainWindowTitle = NULL ) ;
private: // methods
    /**
     * instantiates TCL interpreter and loads tk
     * @return interpreter or NULL
     */
    Tcl_Interp* initInterpreter() ;
    /**
     * Tcl/Tk toolkit init
     * @return true on success
     */
    bool  initTk(Tcl_Interp* aInterp) ;
    /**
     * Initialize extensions to TCL tk provided by classified ads.
     * This includes accessor commands to CA data like articles,
     * profiles and messages.
     *
     * @return true on success
     */
    bool  initExtensions(Tcl_Interp* aInterp) ;

    /**
     * Evaluates in safe interpreter the program given by method
     * @ref setScript.
     *
     * @return true on success
     */
    bool  initProgram(Tcl_Interp* aInterp) ;

    /**
     * Tcl extension method: when TCL scripts asks for list of
     * profiles or classified ads or profile comments, it invokes
     * this method. See @initExtensions
     * where this method is added into TCL interpreters repertoire
     * @param aCData clientdata from tcl interpreter
     * @param aInterp pointer to calling TCL interpreter
     * @param aObjc number of items in array aObjv
     * @param aObjv command arguments. First is name of the command
     *        itself, like "listProfiles" or "listAds", 2nd is search
     *        string.
     * @return TCL_OK on success. As side effect it calls Tcl_SetObjResult that
     *         is a list of dictionaries, each dictionary has fingerprint as the
     *         key and displayName as the value. If no object matches, empty list
     *         is returned.
     */
    static int listItemsCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * static method for getting profile details
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which profile to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getProfileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * static method for getting one classified ad
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which ad to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getClassifiedAdCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * static method for getting one profile comment
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which comment to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getProfileCommentCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * static method for getting binary file
     * @param aObjv should contain 40-byte long hash-string in aObjv[1]
     *        that tells which file to fetch.
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getBinaryFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;

    /**
     * static method for getting db record
     * @param aObjv should contain dictionary expressing the search
     *              conditions. Same dictionary keys are used that are 
     *              in use when db record is published from TCL app.
     * @return If db records satisfying the conditions were found from
     *         local storage, they're returned synchronously as return
     *         value to this call, using Tcl_SetObjResult mechanism. 
     *         Search is sent to remote nodes also and as remote nodes
     *         return search results, new db records may be added to 
     *         database and TCL app running may receive notifications
     *         later concerning db records originally queries but
     *         arriving later. 
     * @param return TCL_OK if ok, the actual TCL object returned is a TCL dictionary.
     */
    static int getDbRecordCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;


    /**
     * Tcl extension method for publishing an item. This will be
     * called by several in-TCL commands, for instance "publishFile",
     * "publishComment" or "publishProfile". The TCL command
     * will be in aObjv[0] and aObjv[1] will contain the actual
     * object to be published.
     * @param aCData clientdata from tcl interpreter. Not used.
     * @param aInterp pointer to calling TCL interpreter
     * @param aObjc number of items in array aObjv
     * @param aObjv command arguments. First is name of the command
     *        itself, like "publishFile", 2nd is actual object
     * @return TCL_OK on success.
     */
    static int publishItemCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * method for calculating SHA1 over a string, callable from TCL
     */
    static int sha1Cmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * method for storing locally a bytearray from TCL
     */
    static int storeTCLProgLocalDataCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * method for retrieving a data-blob previously stored using @ref storeTCLProgLocalData.
     */
    static int retrieveTCLProgLocalDataCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * callback-method for saving a file to local filesystem from 
     * tcl program. Method will open file selection dialog so user can always
     * cancel operation. 
     */
    static int saveFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * Callback-method for reading a file from local filesystem.
     * Method will open file selection dialog so user can always
     * cancel operation. 
     */
    static int openFileCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * Callback-method for checking profile in trust-tree. 
     */
    static int isProfileTrustedCmd(ClientData aCData, Tcl_Interp *aInterp, int aObjc, Tcl_Obj *const aObjv[]) ;
    /**
     * tcl channel procedures
     */
    static int closeProc(
        ClientData aInstanceData,
        Tcl_Interp *aInterp);
    /**
     * this method may be used to write data to tcl interpreter.
     */
    static int inputProc(
        ClientData aInstanceData,
        char *aBuf,
        int aBufSize,
        int *aErrorCodePtr);
    /**
     * output from tcl interpreter comes to this method. See
     * @ref outputProcImpl non-static version of this method
     */
    static int outputProc(
        ClientData aInstanceData,
        const char *aBuf,
        int aToWrite,
        int *aErrorCodePtr);

    static void watchProc(
        ClientData instanceData,
        int mask);
    static int getHandleProc(
        ClientData aInstanceData,
        int aDirection,
        ClientData *aHandlePtr);

    /**
     * output from tcl interpreter comes to this method
     */
    int outputProcImpl(
        ClientData aInstanceData,
        const char *aBuf,
        int aToWrite,
        int *aErrorCodePtr);

    /**
     * method that checks if there is anything in
     * iAddedDataModelItems and if yes, calls notify procedure
     * inside interpreter. Interpreter must have notify
     * procedure installed. See also @ref notifyOfContentReceived.
     * When this is called, there must be at least one item in
     * iAddedDataModelItems
     */
    void notifyInterpreterOfContentReceived( Tcl_Interp* aInterp ) ;
private: // these are not public
    Model& iModel ; /**< datamodel reference */
    MController& iController ;
    Tcl_Interp *iInterp  ; /**< actual TCL interpreter */
    QString iTCLScript ; /**< TCL script to interpret */
    bool iNeedsToRun ; /**< when set to false, will terminate TCL script */
    QByteArray iStdOutBuffer ;
    bool iDeleteLater ; /**< When set to true, will call deleteLater() */
    TclConsoleDialog* iConsole ;
    QQueue<QString> iScriptQueue ;
    QString iMainWindowTitle ;
    QQueue<QPair<Hash, ProtocolItemType> > iAddedDataModelItems ;
    TclCallbacks* iTclCallbacks ;
} ;
#endif
