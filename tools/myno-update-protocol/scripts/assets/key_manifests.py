import struct

# Specifies order of fields (dictionary keys are not necessarily ordered correctly)
key_manifest_update_server_fields = ["app_id", "version", "new_keyinfo", "new_key", "nonce", "keyinfo", "signature"]
key_manifest_vendor_fields = ["app_id", "version", "new_keyinfo", "new_key", "inner_keyinfo", "inner_signature", "nonce", "outer_keyinfo", "outer_signature"]

key_manifest_update_server = {
    "app_id": 0,
    "version": 2,
    "new_keyinfo": struct.pack("<IiIi", 0xFEEDBEEF,-7,2,1), # ES256 (ECDSA w/ SHA-256), EC2, P-256 (secp256r1)
    "new_key": bytes.fromhex("c3b993f64651add0f13cdd98824f04aca351b8349a25194f8b2e1b59ad9a163d2b8732c822c533c7c2ca19d6192062ef9598cbd6759bd58f77ac2fae8970fc6f"),
    "nonce": 1234567890,
    "keyinfo": struct.pack("<IiIi", 0xDEADBEEF,-7,2,1), # ES256 (ECDSA w/ SHA-256), EC2, P-256 (secp256r1)
    # "signature": bytes.fromhex("50c80cc699c782977b067f4825533fd3d6a7b6e00fa1728147945ac64b050b1141eb5f2e153926599fd78e3514b7b8b34662804ed096d14ad5e3b84e9af1319e")
}

key_manifest_vendor = {
    "app_id": "APP",
    "version": 2,
    "new_keyinfo": struct.pack("<IiIi", 0xC007CAFE,-7,2,1),
    "new_key": bytes.fromhex("c666bf9467484c4d6b19a7cee5c532d732670b4bbea4527b83617400df7bc8e74df0c6d506cba61d578d74f631dcad9e7eca9207b3580d1eed60640faaa32214"),
    "inner_keyinfo": struct.pack("IiIi", 0xCAFECAFE,-7,2,1), # ES256 (ECDSA w/ SHA-256), EC2, P-256 (secp256r1)
    # "inner_signature": bytes.fromhex("ddebd613df040716f3a78019ae0f89cd937738ac93c322644fce9c026fe96d3ad29bf728e30c213ecc36ca3b172481cc54a80b1df0bdc463c1248edccbab17eb"),
    "nonce": 1234567890,
    "outer_keyinfo": struct.pack("IiIi", 0xFEEDBEEF,-7,2,1), # ES256 (ECDSA w/ SHA-256), EC2, P-256 (secp256r1)
    # "outer_signature": bytes.fromhex("6edf724a1d8544c4494ec00f9cea4aecd5c1c714c110b8db162dac42bf11cf6ee2678697457daa3335332b3366ba46cfa750a768573fb25856caa328bfa76e96")
}