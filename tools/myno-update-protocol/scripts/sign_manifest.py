#!/usr/bin/env python3

import struct

from assets.manifests import basic_manifest as manifest
from assets.keys import sk_update_server, sk_vendor, DEFAULT_KEY_ID_VENDOR, DEFAULT_KEY_ID_UPDATE_SERVER

MANIFEST_DELIMITER = b"," # ASCII symbol used to separate manifest fields

def sign(sk, data):
    signature = b""
    while not signature or MANIFEST_DELIMITER in signature:
        signature = sk.sign(data)
    return signature

def sign_manifest(manifest, key_id_vendor=DEFAULT_KEY_ID_VENDOR, key_id_update_server=DEFAULT_KEY_ID_UPDATE_SERVER):
    # Creating inner signature

    if key_id_vendor != DEFAULT_KEY_ID_VENDOR:
        manifest["inner_keyinfo"] = struct.pack("I", key_id_vendor) + manifest["inner_keyinfo"][4:]

    manifest_inner = struct.pack("32si32siii16s",
                                bytes(str(manifest["app_id"]), "utf-8"),
                                manifest["link_offset"],
                                manifest["hash_update"],
                                manifest["size"],
                                manifest["version"],
                                manifest["old_version"],
                                manifest["inner_keyinfo"]
                                )
    print("Signing inner manifest:\n%s" % manifest_inner.hex())
    manifest["inner_signature"] = sign(sk_vendor[key_id_vendor], manifest_inner)
    print("Inner signature:\n%s" % manifest["inner_signature"].hex())

    # Creating outer signature

    if key_id_update_server != DEFAULT_KEY_ID_UPDATE_SERVER:
        manifest["outer_keyinfo"] = struct.pack("I", key_id_update_server) + manifest["outer_keyinfo"][4:]

    manifest_outer = struct.pack("32si32siii16s64si16s",
                                bytes(str(manifest["app_id"]), "utf-8"),
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
    print("Signing outer manifest:\n%s" % manifest_outer.hex())
    manifest["outer_signature"] = sign(sk_update_server[key_id_update_server], manifest_outer)
    print("Outer signature:\n%s" % manifest["outer_signature"].hex())

    return manifest

def sign_key_manifest_u(manifest, key_id_update_server=DEFAULT_KEY_ID_UPDATE_SERVER):
    if key_id_update_server != DEFAULT_KEY_ID_UPDATE_SERVER:
        manifest["keyinfo"] = struct.pack("I", key_id_update_server) + manifest["keyinfo"][4:]

    manifest_to_sign = struct.pack("32si16s64si16s",
                                    bytes(str(manifest["app_id"]), "utf-8"),
                                    manifest["version"],
                                    manifest["new_keyinfo"],
                                    manifest["new_key"],
                                    manifest["nonce"],
                                    manifest["keyinfo"],
                                    )
    print("Signing key manifest:\n%s" % manifest_to_sign.hex())
    manifest["signature"] = sign(sk_update_server[key_id_update_server], manifest_to_sign)
    print("Signature:\n%s" % manifest["signature"].hex())

    return manifest

def sign_key_manifest_v(manifest, key_id_vendor=DEFAULT_KEY_ID_VENDOR, key_id_update_server=DEFAULT_KEY_ID_UPDATE_SERVER):
    if key_id_vendor != DEFAULT_KEY_ID_VENDOR:
        manifest["inner_keyinfo"] = struct.pack("I", key_id_vendor) + manifest["inner_keyinfo"][4:]

    manifest_inner = struct.pack("32si16s64s16s",
                                bytes(str(manifest["app_id"]), "utf-8"),
                                manifest["version"],
                                manifest["new_keyinfo"],
                                manifest["new_key"],
                                manifest["inner_keyinfo"]
                                )
    print("Signing inner key manifest:\n%s" % manifest_inner.hex())
    manifest["inner_signature"] = sign(sk_vendor[key_id_vendor], manifest_inner)
    print("Inner signature:\n%s" % manifest["inner_signature"].hex())

    if key_id_update_server != DEFAULT_KEY_ID_UPDATE_SERVER:
        manifest["outer_keyinfo"] = struct.pack("I", key_id_update_server) + manifest["outer_keyinfo"][4:]

    manifest_outer = struct.pack("32si16s64s16s64si16s",
                                bytes(str(manifest["app_id"]), "utf-8"),
                                manifest["version"],
                                manifest["new_keyinfo"],
                                manifest["new_key"],
                                manifest["inner_keyinfo"],
                                manifest["inner_signature"],
                                manifest["nonce"],
                                manifest["outer_keyinfo"]
                                )
    print("Signing outer key manifest:\n%s" % manifest_outer.hex())
    manifest["outer_signature"] = sign(sk_update_server[key_id_update_server], manifest_outer)
    print("Outer signature:\n%s" % manifest["outer_signature"].hex())

    return manifest

if __name__ == "__main__":
    manifest = sign_manifest(manifest)