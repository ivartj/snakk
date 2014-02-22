Basic Idea
==========

The main goal is to create a versatile platform for a variety of
decentralized social media, independent of a central authority for
operation, administration or moderation of the network. Individual
users should be empowered with the ability to extend the platform in
order to shape their online community.

The idea is to create software for query and peering of digitally
signed 'messages', that may be used in a variety of applications. If
they form a forum, a message may represent a post, a reply, a
revision, a tagging, a "Like", or perhaps a "Dislike". Moderation can
be a collaborative process through a network of trust.


## Snakk Message Format

The format starts out in the style of NNTP/HTTP/SMTP/etc
headers. Messages are identified by their content, typically
represented by hash value. A message ID is in the form of

    scheme ":" parameter

For example,

    sha256:d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592

An `Identity` field specifies the method of authentication for the
message, and the parameters to this method that are constant for the
identity that published the message. `Signature` specifies additional
parameters to this method that are specific to the message.

For example, if a message is authenticated by a GPG public key

    Signature: gpg-detached-signature
	sha256:14c7c0d60d938c9dc152a57f1eb3753e3aa94797d7b16f490843b65caa187d22
    Identity: gpg-public-key
	sha256:58694af43c75b1fe02da96d572e835e92ebc286a856835d49bad3b7c1bca2a70
    Content-Type: text/plain

    Hello.

## Command-Line Interface

    snakk verify <message-file>

Checks whether there is a digital signature in the message that can be
verified.

    snakk sign <unsigned-message-file>

Prepend digital signature to file which may include headers.

    snakk publish <message-file>

Publish message file to configured message pools.
Signs message if it is not done.

    snakk extract <message-file> [ -o <output-file> ]

Removes signature and headers and outputs the contained content.

## TODO / Plan

 * Message format specification
 * Example message files conforming to the specification
 * CLI user tools
    * Message parser, printer and manipulation API
    * GPG signatures and verifcation
 * Web message pool
    * Capability to receive messages and maybe also authenticate GPG signatures
    * Allow for query of messages with the following minimum querying
      capabilities:
       * Query latest messages
       * Query messages by a hash value
       * Query messages from identity
       * Query messages in which the Reply-To header has a specified value
       * Query messages with a given word in the Tags header
