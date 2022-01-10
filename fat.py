#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import time
import hashlib
import base64
import configparser
import binascii

# Import pynrfjprog API module and HEX parser module
# from pynrfjprog import Hex,LowLevel

from Crypto.Cipher import AES
from OpenSSL import crypto

from libs.common import *
from libs.vsom_bd import *


def read_file_data(file_name, align=4):
    if file_name[-4:] == ".bin":
        file_type = 1
    elif file_name[-4:] == ".hex":
        file_type = 2
    elif file_name[-4:] == ".imx":
        file_type = 1
    else:
        print("params error")
        return None, None, None, -1

    if file_type == 1:
        file_data = string_to_list(file_readf(file_name))
    else:
        print("File format not support")
        return None, None, None, -1
        '''
        try:
            hex_file = Hex.Hex(file_name)
        except Exception as e:
            raise e

        file_data = []
        for segment in hex_file:
            file_data += segment.data
        '''
    file_size = len(file_data)
    if align > 0:
        # align
        if file_size % align:
            left = align - file_size % align
            file_data += [0xff]*left
            file_size += left

    return file_type, file_size, file_data, 0


def ota_strip(in_data):
    return in_data.replace('\"', '')


def ota_parse_version(in_ver):
    if in_ver is None or (in_ver[0] != r'v' and in_ver[0] != r'V'):
        print("Version format error!")
        raise Exception("Version format error!")

    ver_list = in_ver[1:].split('.')
    if len(ver_list) != 4:
        print("Version format error!")
        raise Exception("Version format error!")

    ver_int = [int(x) for x in ver_list]
    ver_int.reverse()
    return list_to_int(ver_int)


def ota_package_encrypt(file_name, en_key, nonce, init_val, do_en=1):
    if not os.path.exists(file_name):
        print(file_name)
        raise Exception("File not exists!")

    file_type, file_size, file_data, ret = read_file_data(file_name, align=16)
    if ret:
        print("read file error!")
        return -1

    try:
        if do_en:
            cipher = AES.new(bytes(en_key), AES.MODE_CTR, nonce=bytes(nonce), initial_value=init_val)
            file_encrypt = cipher.encrypt(bytes(file_data))
        else:	
            file_encrypt = bytes(file_data)

        return file_encrypt
    except Exception as e:
        print(e)
        return None


def check_images(config):
    temp_folder_path = r"C:\tempFolder"
    Platform_type = config.getint('ProjectConfig', 'Platform_type')
    if Platform_type != 1:
        print("Platform_type error!")
        return -1

    # L2

    # AppKicker
    kicker_file_name = temp_folder_path + r"\app_kicker_nopadding.imx"
    kicker_check_version = ota_parse_version(config.get('App_kicker', 'FWversion'))

    head_data = file_readf(kicker_file_name, r_len=0x1420)
    kicker_version_list = list(head_data[0x1400:0x1404])
    kicker_version_list.reverse()
    kicker_version = list_to_int(kicker_version_list)
    if kicker_check_version != kicker_version:
        print("Check kicker version fail!")
        print("Expect:" + hex(kicker_version) + " but get:" + hex(kicker_check_version))
        return -1

    # App image
    app_file_name = config.get('App', 'file')
    app_check_version = ota_parse_version(config.get('App', 'FWversion'))

    head_data = file_readf(app_file_name, r_len=0x420)
    app_version_list = list(head_data[0x400:0x404])
    app_version_list.reverse()
    app_version = list_to_int(app_version_list)
    if app_check_version != app_version:
        print("Check APP version fail!")
        print("Expect:" + hex(app_version) + " but get:" + hex(app_check_version))
        return -1

    # FCT image
    fct_file_name = config.get('FCT', 'file')
    fct_check_version = ota_parse_version(config.get('FCT', 'FWversion'))

    head_data = file_readf(fct_file_name, r_len=0x420)
    fct_version_list = list(head_data[0x400:0x404])
    fct_version_list.reverse()
    fct_version = list_to_int(fct_version_list)
    if fct_check_version != fct_version:
        print("Check FCT version fail!")
        print("Expect:" + hex(fct_version) + " but get:" + hex(fct_check_version))
        return -1

    # OTA

    # FCT

    return 0

	
def gen_efuse_image(config):
    nxp_tool_path = config.get('ProjectConfig', 'NXP_tool_path')
    factory_file_path = config.get('ProjectConfig', 'Base_path') + \
                        config.get('ProjectConfig', 'Factory_Image_output_path')

    # generate bd file
    fuse_file = None
    if config.has_option('Efuse', 'SRKFuseFile'):
        fuse_file = config.get('Efuse', 'SRKFuseFile')
    close_board = config.getint('Efuse', 'CloseBoard')
    key_setting = 0
    key_05_setting = 0
    bee_key0_source = config.getint('Efuse', 'BEE_key0_source')
    bee_key1_source = config.getint('Efuse', 'BEE_key1_source')
    bt_fuse_sel = config.getint('Efuse', 'BT_FUSE_SEL')
    sjc_disable = config.getint('Efuse', 'SJC_DISABLE')
    encrypted_xip = config.getint('Efuse', 'ENCRYPTED_XIP')
    secondary_pinmux = 0
    if config.has_option('Efuse', 'SECONDARY_PINMUX'):
        secondary_pinmux = config.getint('Efuse', 'SECONDARY_PINMUX')
    key_data = 0
    key_05_data = 0
    comment_str = ""
    comment_05_str = ""
    if bee_key0_source:
        key_setting = 1
        key_data = key_data ^ int("0x00002000",16)
        comment_str = comment_str + "BEE_KEY0_SEL, "
    if bee_key1_source:
        key_setting = 1
        key_data = key_data ^ int("0x00008000",16)
        comment_str = comment_str + "BEE_KEY1_SEL, "
    if bt_fuse_sel:
        key_setting = 1
        key_data = key_data ^ int("0x00000010",16)
        comment_str = comment_str + "BT_FUSE_SEL, "
    if sjc_disable:
        key_setting = 1
        key_data = key_data ^ int("0x00100000",16)
        comment_str = comment_str + "SJC_DISABLE, "
    if close_board:
        key_setting = 1
        key_data = key_data ^ int("0x00000002",16)
        comment_str = comment_str + "CloseBoard, "
    comment_str = comment_str.rstrip(", ")
    key_data = hex(key_data).replace("0x","")
    key_data = "0x"+ key_data.zfill(8) # fill till 4 bytes
    comment_str = "Program " + comment_str + " to use OTPMK"

    if encrypted_xip:
        key_05_setting = 1
        key_05_data = key_05_data ^ int("0x00000002",16)
        comment_05_str = comment_05_str + "ENCRYPTED_XIP, "
    if secondary_pinmux:
        key_05_setting = 1
        key_05_data = key_05_data ^ int("0x00000700",16)
        comment_05_str = comment_05_str + "SECONDARY_PINMUX, "
    comment_05_str = comment_05_str.rstrip(", ")
    key_05_data = hex(key_05_data).replace("0x", "")
    key_05_data = "0x" + key_05_data.zfill(8)  # fill till 4 bytes
    comment_05_str = "Program " + comment_05_str

    bd_file = factory_file_path + r"\hab_efuse.bd"

    vsom_bd = VsomBd(bd_file)
    vsom_bd.init_section(0)

    if fuse_file is not None:
        if not os.path.exists(fuse_file):
            print(fuse_file)
            raise Exception("File not exists!")

        fuse_data = file_readf(fuse_file)
        fuse_word0 = list_to_int(fuse_data[0:4])
        fuse_word1 = list_to_int(fuse_data[4:8])
        fuse_word2 = list_to_int(fuse_data[8:12])
        fuse_word3 = list_to_int(fuse_data[12:16])
        fuse_word4 = list_to_int(fuse_data[16:20])
        fuse_word5 = list_to_int(fuse_data[20:24])
        fuse_word6 = list_to_int(fuse_data[24:28])
        fuse_word7 = list_to_int(fuse_data[28:32])
        vsom_bd.add_section_item("Program SRK table", VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word0), 0x18])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word1), 0x19])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word2), 0x1A])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word3), 0x1B])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word4), 0x1C])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word5), 0x1D])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word6), 0x1E])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["fuse " + hex(fuse_word7), 0x1F])

    if key_setting:
        vsom_bd.add_section_item(comment_str, VsomBd.CMD_LOAD,
                                                    ["fuse " + key_data, 0x06])
    if key_05_setting:
        vsom_bd.add_section_item(comment_05_str, VsomBd.CMD_LOAD,
                                                    ["fuse " + key_05_data, 0x05])
    vsom_bd.write_to_file()

    # genrate sb file
    gen_image = nxp_tool_path + r"\elftosb.exe -f kinetis -V -c "
    gen_image += bd_file
    gen_image += r" -o " + factory_file_path + r"\hab_efuse.sb"

    r_v = os.system(gen_image)
    if r_v:
        print("Generate image error!")


def get_public_key_from_crt(crt_file_name):
    file_data = file_readf(crt_file_name)
    crt_data = crypto.load_certificate(crypto.FILETYPE_PEM, file_data)

    pub_key = crt_data.get_pubkey()
    key_data = list(crypto.dump_publickey(crypto.FILETYPE_ASN1, pub_key))
    key_size = pub_key.bits() * 2 // 8

    return key_data[-key_size:]


def gen_factory_sscz(config):
    temp_folder_path = r"C:\tempFolder"

    nxp_tool_path = config.get('ProjectConfig', 'NXP_tool_path')
    factory_file_path = config.get('ProjectConfig', 'Base_path') + \
                        config.get('ProjectConfig', 'Factory_Image_output_path')
    ECC_signing_certificate_file = config.get('ProjectConfig', 'ECC_signing_certificate_file')
    prod_ca_certificate_file = config.get('ProjectConfig', 'Base_path') + \
                                config.get('ProjectConfig', 'ProdCaCertificate')
    prod_ica_certificate_file = config.get('ProjectConfig', 'Base_path') + \
                                 config.get('ProjectConfig', 'ProdICACertificate')

    image_en = config.getint('ProjectConfig', 'Enc_Factory_Deliver')

    # 1. App_config
    config_start_address = int(config.get('App_config', 'DestAddress'), 16)
    org_config_file_name = config.get('App_config', 'file')

    config_output_file = temp_folder_path + r"\config_encrypt.bin"
    bd_file = factory_file_path + r"\factory_config.bd"

    if not os.path.exists(ECC_signing_certificate_file):
        print(ECC_signing_certificate_file)
        raise Exception("File not exists!")
    if not os.path.exists(org_config_file_name):
        print(org_config_file_name)
        raise Exception("File not exists!")
    if not os.path.exists(prod_ca_certificate_file):
        print(prod_ca_certificate_file)
        raise Exception("File not exists!")
    if not os.path.exists(prod_ica_certificate_file):
        print(prod_ica_certificate_file)
        raise Exception("File not exists!")
    config_file_size = os.path.getsize(org_config_file_name)
    org_config_data = list(file_readf(org_config_file_name))

    # Update public key
    config_file_name = temp_folder_path + r"\config_sscz.bin"

    if config.getint('ProjectConfig', 'Enable_RootKEY'):
        root_key_file = config.get('ProjectConfig', 'RootKey_file')
        if not os.path.exists(root_key_file):
            print(root_key_file)
            raise Exception("File not exists!")
        root_key_read = file_read_bin(root_key_file, 32)
        print('Root key is:')
        print(root_key_read)
        root_key_data = str(root_key_read).replace('b\'', '').replace('\'', '')
        print(root_key_data)
        root_key_data = ota_strip(config.get('ProjectConfig', 'RootKey'))
        if len(root_key_data) != 64:
            print("RootKey length error!")
            return -1
        root_key = list(bytes.fromhex(root_key_data))
        print(root_key)
        org_config_data[0xc000:0xc000 + len(root_key)] = root_key

    ecc_public_key = get_public_key_from_crt(ECC_signing_certificate_file)
    org_config_data[0xf000:0xf000+len(ecc_public_key)] = ecc_public_key

    prod_ca_certificate_file_read = file_read_bin(prod_ca_certificate_file)
    print('ProdCaCertificate is:')
    print(prod_ca_certificate_file_read)
    prod_ca_certificate_file_data = str(prod_ca_certificate_file_read).replace('b\'', '').replace('\'', '')
    print(prod_ca_certificate_file_data)
    prod_ca_certificate_file_data = ota_strip(prod_ca_certificate_file_data)
    prod_ca_certificate = list(bytes.fromhex(prod_ca_certificate_file_data))
    org_config_data[0xd000:0xd000 + len(prod_ca_certificate)] = prod_ca_certificate

    prod_ica_certificate_file_read = file_read_bin(prod_ica_certificate_file)
    print('ProdICACertificate is:')
    print(prod_ica_certificate_file_read)
    prod_ica_certificate_file_data = str(prod_ica_certificate_file_read).replace('b\'', '').replace('\'', '')
    print(prod_ica_certificate_file_data)
    prod_ica_certificate_file_data = ota_strip(prod_ica_certificate_file_data)
    prod_ica_certificate = list(bytes.fromhex(prod_ica_certificate_file_data))
    org_config_data[0xe000:0xe000 + len(prod_ica_certificate)] = prod_ica_certificate

    # Update product ID and hardware version
    productID = int(ota_strip(config.get('App', 'productID')))
    HWversion = int(ota_strip(config.get('App', 'HWversion')))
    productID_list = int_to_list(productID)
    HWversion_list = int_to_list(HWversion)
    Project_Name = config.get('ProjectConfig', 'Project_Name')
    Project_Name_list = string_to_list(Project_Name)

    org_config_data[0:4] = HWversion_list
    org_config_data[4:8] = productID_list
    org_config_data[0x20:0x20+len(Project_Name_list)] = Project_Name_list

    # Update Appkicker flags
    kicker_status = [0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00]
    kicker_version = ota_parse_version(config.get('App_kicker', 'FWversion'))
    kicker_version_list = int_to_list_be(kicker_version)

    org_config_data[0x1000:0x1000+len(kicker_status)] = kicker_status
    org_config_data[0x2000:0x2000+len(kicker_version_list)] = kicker_version_list
    org_config_data[0x3000:0x3000 + len(kicker_version_list)] = kicker_version_list

    # Update OTA flags
    ota_status = [0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00]
    ota_version = ota_parse_version(config.get('ProjectConfig', 'OTA_version'))
    ota_version_list = int_to_list_be(ota_version)

    org_config_data[0x4000:0x4000 + len(ota_status)] = ota_status
    org_config_data[0x5000:0x5000 + len(ota_version_list)] = ota_version_list
    org_config_data[0x6000:0x6000 + len(ota_version_list)] = ota_version_list

    # Update L2 flags
    l2_version = ota_parse_version(config.get('L2', 'FWversion'))
    l2_version_list = int_to_list_be(l2_version)
    org_config_data[0x13000:0x13000 + len(l2_version_list)] = l2_version_list

    # Update App flags
    app_status = [0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00]
    app_version = ota_parse_version(config.get('App', 'FWversion'))
    fct_version = ota_parse_version(config.get('FCT', 'FWversion'))
    app_version_list = int_to_list_be(app_version)
    fct_version_list = int_to_list_be(fct_version)

    org_config_data[0x7000:0x7000 + len(app_status)] = app_status
    org_config_data[0x8000:0x8000 + len(app_version_list)] = fct_version_list
    org_config_data[0x9000:0x9000 + len(fct_version_list)] = app_version_list

    file_writef(config_file_name, org_config_data)
    time.sleep(0.1)

    image_en_nonce = get_random_vector(12)
    if image_en:
        deliver_key_file = config.get('ProjectConfig', 'EncFactoryDeliverKey_file')
        if not os.path.exists(deliver_key_file):
            print(deliver_key_file)
            raise Exception("File not exists!")
        image_en_key_read = file_read_bin(deliver_key_file, 16)
        print(image_en_key_read)
        image_en_key = str(image_en_key_read).replace('b\'', '').replace('\'','')
        image_en_key = ota_strip(config.get('ProjectConfig', 'EncFactoryDeliverKey'))
        print(image_en_key)
        if len(image_en_key) != 32:
            print("EncFactoryDeliverKey error!")
            return -1

        # encrypt image
        app_en_key = list(bytes.fromhex(image_en_key))
        print(app_en_key)

        address_ctr = config_start_address
        address_ctr = address_ctr >> 4
        config_file_encrypt = ota_package_encrypt(config_file_name,
                                                  app_en_key,
                                                  image_en_nonce,
                                                  address_ctr,
                                                  image_en)
        if config_file_encrypt is None:
            print("Encrypt Config error!")
            return -1

        file_writef(config_output_file, config_file_encrypt)
        print("Encrypt Config size:   " + hex(len(config_file_encrypt)))

    # generate bd file
    vsom_bd = VsomBd(bd_file)
    vsom_bd.init_section(0)
    vsom_bd.add_source("ConfigFile", None)

    if image_en:
        vsom_bd.add_section_item("Set SSCZ address", VsomBd.CMD_LOAD,
                                                        [0x53530c01, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [config_start_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD,
                                        [config_start_address + config_file_size, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

        vsom_bd.add_section_item("Set Nonce", VsomBd.CMD_LOAD,
                                                    [0x53530b00, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [list_to_int(image_en_nonce[0:4]), 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [list_to_int(image_en_nonce[4:8]), 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [list_to_int(image_en_nonce[8:12]), 0x400c])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    vsom_bd.add_section_item("Program Config", VsomBd.CMD_ERASE,
                             [config_start_address, config_start_address + config_file_size])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["ConfigFile", config_start_address])

    vsom_bd.write_to_file()

    # genrate sb file
    gen_image = nxp_tool_path + r"\elftosb.exe -f kinetis -V -c "
    gen_image += bd_file
    gen_image += r" -o " + factory_file_path + r"\factory_config.sb "
    if image_en:
        gen_image += config_output_file
    else:
        gen_image += config_file_name

    r_v = os.system(gen_image)
    if r_v:
        print("Generate image error!")
        return -1

    # Clear files
    if os.path.exists(config_output_file):
        os.remove(config_output_file)

    return 0


def get_file_info(config, customized_section, file, addr):
    customized_section_file = config.get(customized_section, file)
    if not os.path.exists(customized_section_file):
        print(customized_section_file)
        raise Exception("File not exists!")
    customized_section_file_size = os.path.getsize(customized_section_file)
    customized_section_addr = int(config.get(customized_section, addr), 16)
    return customized_section_file, customized_section_file_size, customized_section_addr


def add_customized_section(vsom_bd, VsomBd, program_content, customized_section, start_addr, size):
    vsom_bd.add_section_item(program_content, VsomBd.CMD_ERASE,
                             [start_addr, start_addr + size])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [customized_section, start_addr])
    return 0

def gen_factory_image(config):
    temp_folder_path = r"C:\tempFolder"

    nxp_tool_path = config.get('ProjectConfig', 'NXP_tool_path')
    factory_file_path = config.get('ProjectConfig', 'Base_path') + \
                        config.get('ProjectConfig', 'Factory_Image_output_path')
    image_en = config.getint('ProjectConfig', 'Enc_Factory_Deliver')

    l2_start_address = int(config.get('L2', 'DestAddress'), 16)
    app_start_address = int(config.get('ProjectConfig', 'OTA_Primary_address'), 16)
    app_start2_address = int(config.get('ProjectConfig', 'OTA_Backup_address'), 16)
    app_zone_size = int(config.get('ProjectConfig', 'OTA_Zone_size'), 16)
    kicker_start_address = int(config.get('ProjectConfig', 'Kicker_Primary_address'), 16)
    kicker_start2_address = int(config.get('ProjectConfig', 'Kicker_Backup_address'), 16)
    fct_start_address = app_start2_address
    data_start_address = -1
    data_end_address = -1
    if config.has_option('ProjectConfig', 'DATA_START'):
        data_start_address = int(config.get('ProjectConfig', 'DATA_START'), 16)
    if config.has_option('ProjectConfig', 'DATA_END'):
        data_end_address = int(config.get('ProjectConfig', 'DATA_END'), 16)

    FcbFile = config.get('ProjectConfig', 'FcbFile')
    l2_file_name = temp_folder_path + r"\l2_nopadding.imx"
    kicker_file_name = temp_folder_path + r"\app_kicker_nopadding.imx"
    app_file_name = temp_folder_path + r"\hybird_app_signed.bin"
    fct_file_name = temp_folder_path + r"\hybird_fct_signed.bin"

    bd_file = factory_file_path + r"\factory_image.bd"
    l2_output_file = temp_folder_path + r"\l2_image_encrypt.bin"
    kicker_output_file = temp_folder_path + r"\kicker_image_encrypt.bin"
    kicker2_output_file = temp_folder_path + r"\kicker2_image_encrypt.bin"
    app_output_file = temp_folder_path + r"\app_image_encrypt.bin"
    fct_output_file = temp_folder_path + r"\fct_image_encrypt.bin"

    if not os.path.exists(FcbFile):
        print(FcbFile)
        raise Exception("File not exists!")
    if not os.path.exists(l2_file_name):
        print(l2_file_name)
        raise Exception("File not exists!")
    if not os.path.exists(kicker_file_name):
        print(kicker_file_name)
        raise Exception("File not exists!")

    if not os.path.exists(app_file_name):
        print(app_file_name)
        raise Exception("File not exists!")
    app_file_size = os.path.getsize(app_file_name)

    if not os.path.exists(fct_file_name):
        print(fct_file_name)
        raise Exception("File not exists!")
    fct_file_size = os.path.getsize(fct_file_name)

    customizedSectionNum = config.getint('ProjectConfig', 'Customized_Section_Num')
    if customizedSectionNum > 10 or customizedSectionNum < 0:
        raise Exception("The number of customer is error! need 0 <= Customized_Section_Num <= 10")

    customized_section_file = []
    customized_section_file_size = []
    customized_section_addr = []

    for i in range(customizedSectionNum):
        customizedSectionFile, customizedSectionFileSize, customizedSectionAddr = \
            get_file_info(config, 'Customized_Section' + str(i + 1), 'file', 'DestAddress')
        customized_section_file.append(customizedSectionFile)
        customized_section_file_size.append(customizedSectionFileSize)
        customized_section_addr.append(customizedSectionAddr)

    image_en_nonce = get_random_vector(12)
    if image_en:
        deliver_key_file = config.get('ProjectConfig', 'EncFactoryDeliverKey_file')
        if not os.path.exists(deliver_key_file):
            print(deliver_key_file)
            raise Exception("File not exists!")
        deliver_key = file_read_bin(deliver_key_file, 16)
        print('EncFactoryDeliverKey is:')
        print(deliver_key)
        image_en_key = str(deliver_key).replace('b\'', '').replace('\'','')
        image_en_key = ota_strip(config.get('ProjectConfig', 'EncFactoryDeliverKey'))
        print(image_en_key)
        if len(image_en_key) != 32:
            print("EncFactoryDeliverKey error!")
            return -1

        # encrypt image
        app_en_key = list(bytes.fromhex(image_en_key))
        print(app_en_key)
        # 1.L2
        address_ctr = l2_start_address
        address_ctr = address_ctr >> 4
        l2_file_encrypt = ota_package_encrypt(l2_file_name,
                                              app_en_key,
                                              image_en_nonce,
                                              address_ctr,
                                              image_en)
        if l2_file_encrypt is None:
            print("Encrypt L2 error!")
            return -1

        file_writef(l2_output_file, l2_file_encrypt)
        print("Encrypt L2 size:   " + hex(len(l2_file_encrypt)))

        # 2.AppKicker
        address_ctr = kicker_start_address
        address_ctr = address_ctr >> 4
        kicker_file_encrypt = ota_package_encrypt(kicker_file_name,
                                                  app_en_key,
                                                  image_en_nonce,
                                                  address_ctr,
                                                  image_en)
        if kicker_file_encrypt is None:
            print("Encrypt Kicer error!")
            return -1

        file_writef(kicker_output_file, kicker_file_encrypt)
        print("Encrypt Kicker size:   " + hex(len(kicker_file_encrypt)))

        # kicker2
        address_ctr = kicker_start2_address
        address_ctr = address_ctr >> 4
        kicker2_file_encrypt = ota_package_encrypt(kicker_file_name,
                                                   app_en_key,
                                                   image_en_nonce,
                                                   address_ctr,
                                                   image_en)
        if kicker2_file_encrypt is None:
            print("Encrypt Kicker error!")
            return -1

        file_writef(kicker2_output_file, kicker2_file_encrypt)

        # 3.App
        address_ctr = app_start_address
        address_ctr = address_ctr >> 4
        app_file_encrypt = ota_package_encrypt(app_file_name,
                                               app_en_key,
                                               image_en_nonce,
                                               address_ctr,
                                               image_en)
        if app_file_encrypt is None:
            print("Encrypt App error!")
            return -1

        file_writef(app_output_file, app_file_encrypt)
        print("encrypt app size:   " + hex(len(app_file_encrypt)))

        # 4.FCT
        address_ctr = app_start2_address
        address_ctr = address_ctr >> 4
        fct_file_encrypt = ota_package_encrypt(fct_file_name,
                                               app_en_key,
                                               image_en_nonce,
                                               address_ctr,
                                               image_en)
        if fct_file_encrypt is None:
            print("Encrypt FCT error!")
            return -1

        file_writef(fct_output_file, fct_file_encrypt)
        print("encrypt fct size:   " + hex(len(fct_file_encrypt)))

    # generate bd file
    if config.has_option('ProjectConfig', 'EncryptBoot'):
        EncryptBoot = config.getint('ProjectConfig', 'EncryptBoot')
    else:
        EncryptBoot = 1

    if config.has_option('ProjectConfig', 'FlashRemap'):
        FlashRemap = config.getint('ProjectConfig', 'FlashRemap')
    else:
        FlashRemap = 1

    vsom_bd = VsomBd(bd_file)
    vsom_bd.init_section(0)
    vsom_bd.add_source("FcbFile", None)
    vsom_bd.add_source("L2File", None)
    vsom_bd.add_source("AppKickerPrimary", None)
    vsom_bd.add_source("AppKickerSecondary", None)
    vsom_bd.add_source("AppFilePrimary", None)
    vsom_bd.add_source("AppFileSecondary", None)

    for i in range(customizedSectionNum):
        vsom_bd.add_source("Customized_Section" + str(i + 1), None)

    vsom_bd.add_section_item("Load FCB file to ITCM address 0x2000", VsomBd.CMD_LOAD,
                             ["FcbFile", 0x2000])
    vsom_bd.add_section_item("Configure FLASH using options on address 0x2000",
                             VsomBd.CMD_ENABLE, [r"flexspinor", 0x2000])

    vsom_bd.add_section_item("Erase flash for FCB and L2 - "
                             "from flash base to the beginning of the first partition",
                             VsomBd.CMD_ERASE, [0x60000000, kicker_start_address])

    if FlashRemap:
        vsom_bd.add_section_item("Set Flash remap address", VsomBd.CMD_LOAD,
                                 [0x53530e01, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start2_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start2_address + app_zone_size//2, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address, 0x400c])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    if image_en:
        vsom_bd.add_section_item("Set SSDL address", VsomBd.CMD_LOAD,
                                 [0x53530d01, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [l2_start_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address+app_zone_size, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

        vsom_bd.add_section_item("Set Nonce", VsomBd.CMD_LOAD,
                                 [0x53530b00, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [list_to_int(image_en_nonce[0:4]), 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [list_to_int(image_en_nonce[4:8]), 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [list_to_int(image_en_nonce[8:12]), 0x400c])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    if EncryptBoot:
        # PRDB0
        vsom_bd.add_section_item("Prepare PRDB0 options", VsomBd.CMD_LOAD, [0xe0110000, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [l2_start_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address - l2_start_address, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    vsom_bd.add_section_item("Program config block", VsomBd.CMD_LOAD, [0xf000000f, 0x3000])
    vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x3000])

    vsom_bd.add_section_item("Program L2 image", VsomBd.CMD_LOAD,
                             ["L2File", l2_start_address])

    vsom_bd.add_section_item("Program Kicker", VsomBd.CMD_ERASE,
                             [kicker_start_address, app_start_address])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppKickerPrimary", kicker_start_address])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppKickerSecondary", kicker_start2_address])

    if EncryptBoot:
        # PRDB1
        vsom_bd.add_section_item("Prepare PRDB1 options", VsomBd.CMD_LOAD, [0xe4110000, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_zone_size, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    vsom_bd.add_section_item("Program App", VsomBd.CMD_ERASE,
                             [app_start_address, app_start_address+app_file_size])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppFilePrimary", app_start_address])
    vsom_bd.add_section_item(None, VsomBd.CMD_ERASE,
                             [app_start2_address, app_start2_address + fct_file_size])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppFileSecondary", app_start2_address])

    if data_start_address != -1 and data_end_address != -1:
        vsom_bd.add_section_item("Program Data", VsomBd.CMD_ERASE,
                             [data_start_address, data_end_address])

    for i in range(customizedSectionNum):
        file_size = customized_section_file_size[i]
        if i == 0:
            file_size = 1024*512
        add_customized_section(vsom_bd, VsomBd, "Program Customized_Section" + str(i + 1), "Customized_Section" +
                               str(i + 1), customized_section_addr[i], file_size)
    vsom_bd.write_to_file()

    # genrate sb file
    gen_image = nxp_tool_path + r"\elftosb.exe -f kinetis -V -c "
    gen_image += bd_file
    gen_image += r" -o " + factory_file_path + r"\factory_image.sb "
    gen_image += FcbFile + r" "
    if image_en:
        gen_image += l2_output_file + r" " + \
                     kicker_output_file + r" " + \
                     kicker2_output_file + r" " + \
                     app_output_file + r" " + \
                     fct_output_file
    else:
        gen_image += l2_file_name + r" " + \
                     kicker_file_name + r" " + \
                     kicker_file_name + r" " + \
                     app_file_name + r" " + \
                     fct_file_name

    for i in range(customizedSectionNum):
        gen_image += r" " + customized_section_file[i]

    r_v = os.system(gen_image)
    if r_v:
        print("Generate image error!")
        return -1

    # Clear files
    if os.path.exists(l2_output_file):
        os.remove(l2_output_file)
    if os.path.exists(kicker_output_file):
        os.remove(kicker_output_file)
    if os.path.exists(kicker2_output_file):
        os.remove(kicker2_output_file)
    if os.path.exists(app_output_file):
        os.remove(app_output_file)
    if os.path.exists(fct_output_file):
        os.remove(fct_output_file)

    return 0


def gen_dev_image(config):
    temp_folder_path = r"C:\tempFolder"

    nxp_tool_path = config.get('ProjectConfig', 'NXP_tool_path')
    factory_file_path = config.get('ProjectConfig', 'Base_path') + \
                        config.get('ProjectConfig', 'Factory_Image_output_path')

    l2_start_address = int(config.get('L2', 'DestAddress'), 16)
    app_start_address = int(config.get('ProjectConfig', 'OTA_Primary_address'), 16)
    app_start2_address = int(config.get('ProjectConfig', 'OTA_Backup_address'), 16)
    app_zone_size = int(config.get('ProjectConfig', 'OTA_Zone_size'), 16)
    kicker_start_address = int(config.get('ProjectConfig', 'Kicker_Primary_address'), 16)
    kicker_start2_address = int(config.get('ProjectConfig', 'Kicker_Backup_address'), 16)

    FcbFile = config.get('ProjectConfig', 'FcbFile')
    l2_file_name = temp_folder_path + r"\l2_nopadding.imx"
    kicker_file_name = temp_folder_path + r"\app_kicker_nopadding.imx"
    app_file_name = temp_folder_path + r"\hybird_app_signed.bin"
    fct_file_name = temp_folder_path + r"\hybird_fct_signed.bin"

    bd_file = factory_file_path + r"\dev_image.bd"

    if not os.path.exists(app_file_name):
        print(app_file_name)
        raise Exception("File not exists!")
    app_file_size = os.path.getsize(app_file_name)

    if not os.path.exists(fct_file_name):
        print(fct_file_name)
        raise Exception("File not exists!")
    fct_file_size = os.path.getsize(fct_file_name)

    # generate bd file
    if config.has_option('ProjectConfig', 'EncryptBoot'):
        EncryptBoot = config.getint('ProjectConfig', 'EncryptBoot')
    else:
        EncryptBoot = 1

    if config.has_option('ProjectConfig', 'FlashRemap'):
        FlashRemap = config.getint('ProjectConfig', 'FlashRemap')
    else:
        FlashRemap = 1

    vsom_bd = VsomBd(bd_file)
    vsom_bd.init_section(0)
    vsom_bd.add_source("FcbFile", None)
    vsom_bd.add_source("L2File", None)
    vsom_bd.add_source("AppKickerPrimary", None)
    vsom_bd.add_source("AppKickerSecondary", None)
    vsom_bd.add_source("AppFilePrimary", None)
    vsom_bd.add_source("AppFileSecondary", None)

    vsom_bd.add_section_item("Load FCB file to ITCM address 0x2000", VsomBd.CMD_LOAD,
                             ["FcbFile", 0x2000])
    vsom_bd.add_section_item("Configure FLASH using options on address 0x2000",
                             VsomBd.CMD_ENABLE, [r"flexspinor", 0x2000])

    vsom_bd.add_section_item("Erase flash for FCB and L2 - "
                             "from flash base to the beginning of the first partition",
                             VsomBd.CMD_ERASE, [0x60000000, kicker_start_address])

    if FlashRemap:
        vsom_bd.add_section_item("Set Flash remap address", VsomBd.CMD_LOAD,
                                 [0x53530e01, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start2_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start2_address + app_zone_size // 2, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address, 0x400c])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    if EncryptBoot:
        # PRDB0
        vsom_bd.add_section_item("Prepare PRDB0 options", VsomBd.CMD_LOAD, [0xe0110000, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [l2_start_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address - l2_start_address, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    vsom_bd.add_section_item("Program config block", VsomBd.CMD_LOAD, [0xf000000f, 0x3000])
    vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x3000])

    vsom_bd.add_section_item("Program L2 image", VsomBd.CMD_LOAD,
                             ["L2File", l2_start_address])

    vsom_bd.add_section_item("Program Kicker", VsomBd.CMD_ERASE,
                             [kicker_start_address, app_start_address])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppKickerPrimary", kicker_start_address])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppKickerSecondary", kicker_start2_address])

    if EncryptBoot:
        # PRDB1
        vsom_bd.add_section_item("Prepare PRDB1 options", VsomBd.CMD_LOAD, [0xe4110000, 0x4000])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_start_address, 0x4004])
        vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, [app_zone_size, 0x4008])
        vsom_bd.add_section_item(None, VsomBd.CMD_ENABLE, [r"flexspinor", 0x4000])

    vsom_bd.add_section_item("Program App", VsomBd.CMD_ERASE,
                             [app_start_address, app_start_address + app_file_size])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppFilePrimary", app_start_address])
    vsom_bd.add_section_item(None, VsomBd.CMD_ERASE,
                             [app_start2_address, app_start2_address + fct_file_size])
    vsom_bd.add_section_item(None, VsomBd.CMD_LOAD, ["AppFileSecondary", app_start2_address])

    vsom_bd.write_to_file()

    # genrate sb file
    gen_image = nxp_tool_path + r"\elftosb.exe -f kinetis -V -c "
    gen_image += bd_file
    gen_image += r" -o " + factory_file_path + r"\dev_image.sb "
    gen_image += FcbFile + r" "

    gen_image += l2_file_name + r" " + \
                 kicker_file_name + r" " + \
                 kicker_file_name + r" " + \
                 app_file_name + r" " + \
                 app_file_name

    r_v = os.system(gen_image)
    if r_v:
        print("Generate image error!")
        return -1

    return 0


def gen_vsom_image(sys_argv):
    '''
    --vsom [config]
    '''
    if len(sys_argv) == 0:
        print("param error")
        return None

    config_file = sys_argv[0]
    if not os.path.exists(config_file):
        print(config_file)
        raise Exception("File not exists!")

    # Parse config
    config = configparser.ConfigParser()
    config.read(config_file)

    # Check image version
    ret = check_images(config)
    if ret:
        print("Images check fail!")
        raise Exception("Fail!")

    # Gen efuse image
    gen_efuse_image(config)

    # Gen factory package
    ret = gen_factory_image(config)
    if ret:
        print("Gen factory image fail!")
        raise Exception("Fail!")

    ret = gen_factory_sscz(config)
    if ret:
        print("Gen config image fail!")
        raise Exception("Fail!")

    # Gen dev image
    ret = gen_dev_image(config)
    if ret:
        print("Gen dev image fail!")
        raise Exception("Fail!")

    print("Run success")

def file_read_bin(path, r_len=0):
    if not os.path.exists(path):
        print(path)
        raise Exception("File not exists!")
    f = open(path, 'rb')
    if r_len > 0:
        r_data = f.read(r_len)
    else:
        r_data = f.read()
    f.close()
    r_data = binascii.b2a_hex(r_data)
    return r_data

def print_help(sys_argv):
    print("\t--vsom <config file>")
    print("\t\t\t\t: Generate SB image")

    print("\t--help")
    print("\t\t\t\t: Print this help")


params_list = [
                ["--vsom", gen_vsom_image],
                ["--help", print_help]
                ]


# Run main.
if __name__ == "__main__":

    print("FAT tool: 0.0.0.2")
    print("Type --help for more information")

    argc = len(sys.argv)

    if argc >= 2:
        params = sys.argv[1]

        if params[0:2] != "--":
            print("params error")
            exit(0)

        for idx in range(len(params_list)):
            if params_list[idx][0] == params:
                params_list[idx][1](sys.argv[2:])
                break
    else:
        print_help([])
