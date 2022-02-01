import struct

basic_manifest = {
    "app_id": "APP",
    "link_offset": 0,
    "hash_update": bytes.fromhex("cc6cf68547fe9a90070da75bdc953f34af1c708d1baf8c2912070d2f05845f73"),
    "size": 1,
    "version": 2,
    "old_version": 1,
    "inner_keyinfo": struct.pack("IiIi", 0xCAFECAFE,-7,2,1), # ES256 (ECDSA w/ SHA-256), EC2, P-256 (secp256r1)
    "inner_signature": bytes.fromhex("88b9bc781008845993de7e57bfcc5f9e2d5a1a195c842682a1f3d9615317b98db497819ae660c43920d929124158bc2d4ac593f08e78ecf9930e3d0eae210e9d"),
    "nonce": 1234567890,
    "outer_keyinfo": struct.pack("IiIi", 0xDEADBEEF,-7,2,1), # ES256 (ECDSA w/ SHA-256), EC2, P-256 (secp256r1)
    "outer_signature": bytes.fromhex("ff85710a8dd71a8d59487490b2c51ca8b2bfb0565824c7d68e00aff3184745a4af6115b5821c1e56c30ff5e46ae05b8ea3adc274e24242a1d80f58b0e7196263")
}