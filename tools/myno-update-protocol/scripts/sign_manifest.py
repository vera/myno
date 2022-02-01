#!/usr/bin/env python3

import struct

from assets.manifests import basic_manifest as manifest
from assets.keys import sk_update_server, sk_vendor

MANIFEST_DELIMITER = b"," # ASCII symbol used to separate manifest fields

def sign(sk, data):
    signature = b""
    while not signature or MANIFEST_DELIMITER in signature:
        signature = sk.sign(data)
    return signature

# Creating inner signature

manifest_inner = struct.pack("32si32siii16s",
                             bytes(manifest["app_id"], "utf-8"),
                             manifest["link_offset"],
                             manifest["hash_update"],
                             manifest["size"],
                             manifest["version"],
                             manifest["old_version"],
                             manifest["inner_keyinfo"]
                            )
print("Signing inner manifest:\n%s" % manifest_inner.hex())
inner_signature = sign(sk_vendor, manifest_inner)
print("Inner signature:\n%s" % inner_signature.hex())

# Creating outer signature

manifest["inner_signature"] = inner_signature
manifest_outer = struct.pack("32si32siii16s64si16s",
                             bytes(manifest["app_id"], "utf-8"),
                             manifest["link_offset"],
                             manifest["hash_update"],
                             manifest["size"],
                             manifest["version"],
                             manifest["old_version"],
                             manifest["inner_keyinfo"],
                             manifest["inner_signature"],
                             manifest["nonce"],
                             manifest["outer_keyinfo"]
                            )
print("\nSigning outer manifest:\n%s" % manifest_outer.hex())
outer_signature = sign(sk_update_server, manifest_outer)
print("Outer signature:\n%s" % outer_signature.hex())