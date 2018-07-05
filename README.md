classified-ads
==============

Classified ads is a server-less program for internet communications, 
including public and private messages. Things you can do with classified-ads
include
 * Posting and reading of public messages, the "classified ads" that 
   don't necessarily need to do anything with selling or buying of 
   things ; consider this thing a public discussion forum.
 * Publishing of small set of information about ourself as user of the system.
 * Leaving public comments about operators ; if operator has decided to 
   mark information regarding himself as private-within-group, then only
   the named group can read the comments. 
 * Sending and receiving of messages between operators that are not 
   intended for others to see.
 * Performing of (word-based) searches of content posted into the system.
 * Doing voip calls between operators in the network.
 * Extending existing functionality with Tcl scripts to build applications inside classified ads that can access resources in classified ads p2p network.
 * Using rudimentary general-purpose distributed database for sharing data between Tcl scripts running on each users nodes.

Server-less means that system has completely distributed design, each 
node in the network implements a "mini-server" that serves other users
in the network in form of data storage and data transfer. System has no
concept of "service provider" or "server" that would later disappear
leaving users without messaging capabilities. For keeping private things
private encryption is applied whenever possible. Visit
http://katiska.org/classified-ads/ for more details. 

Classified-ads is known to work in linux and other unix-like operating-
systems and microsoft windows. Most of functionality is on top of Qt 
library so porting to other environments where Qt is available might be
possible.
