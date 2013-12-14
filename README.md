BASIC IDEA

The main goal is to create a versatile platform for a variety of
decentralized social media, independent of a central authority for
operation, administration or moderation of the network. Individual
users should be empowered with the ability to extend the platform in
order to shape their online community.

The idea is create software for query and peering of digitally signed
'messages', that may be used in a variety of applications. If they
form a forum, a message may represent a post, a reply, a revision, a
tagging, a "Like", or perhaps a "Dislike". Moderation can be a
collaborative process through a network of trust.


SNAKK MESSAGE FORMAT

The format starts out in the style of NNTP/HTTP/SMTP/etc headers. The
standardized headers `Signature`, `Watermark` and `Identity` must
always come before the other headers, in that order. The `Signature`
and `Identity` headers are mandatory, while the `Watermark` header
is not. Other headers may be used for application-specific purposes.

Messages are identified by their content below the Watermark field, or
Signature field if it is not present, typically represented by hash value.
A message ID is in the form of

    scheme ":" parameter

For example,

    sha256:d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592

The `Identity` field specifies the method of authentication for the
message, and the parameters to this method that are constant for this
identity. `Signature` specifies additional parameters to this method
that are specific to the message. The composed method authenticates
everything below the `Signature` field, including the optional
`Watermark` field. The `Watermark` field has an arbitrary value that
can theoretically be used to infer which entities have redistributed a
message.

For example, if a message is authenticated by a GPG public key

    Signature: d7a8fbb307d7809469c
    Watermark: 127890340891
    Identity: gpg-public-key e4f8d5651e46d3cdb762d02d0bf37c9e592

(these aren't actual GPG public keys and signatures)
