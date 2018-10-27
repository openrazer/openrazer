-- Thanks to karlp for making a nice lua protocol example for USB https://github.com/karlp/swopy
--
-- Copyright 2015 Terry Cain <terry@terrys-home.co.uk>
-- To install put in ~/.wireshark/plugins/FILENAME.lua
-- or ran with wireshark -X lua_script:FILENAME.lua
--
-- Could of binded it to the Razer USB Vendor:Product ID but when wiresharking USB comms to virtualbox
-- wireshark can miss the VID,PID declaration and then the dissector wouldn't apply so just use it when
-- dissecting razer traffic, don't use it all the time as it'll apply to all usb-control requests

razer_proto = Proto("razer", "Razer Protocol")

local req_types = {
    [0x00] = "REQUEST",
    [0x02] = "RESPONSE",
}

local f = razer_proto.fields
f.f_reqtype = ProtoField.uint8("razer.request_type", "Request Type", base.HEX, req_types)           -- 1b
f.f_id = ProtoField.uint8("razer.id", "ID", base.HEX)                                               -- 1b Possibly I2C range
f.f_reserved1 = ProtoField.bytes("razer.reserved1", "Reserved 1", base.HEX)                         -- 3b
f.f_number_parameter_bytes = ProtoField.uint8("razer.num_params", "Number of parameters", base.HEX) -- 1b Number of bytes after command byte
f.f_reserved2 = ProtoField.uint8("razer.reserver2", "Reserved 2", base.HEX)                         -- 1b Possibly I2C range also
f.f_command = ProtoField.uint8("razer.command", "Command", base.HEX)                                -- 1b Command byte
f.f_sub_command = ProtoField.uint8("razer.sub_command", "Sub Command", base.HEX)                    -- 1b Sub Command byte
f.f_command_params = ProtoField.bytes("razer.command_params", "Command parameters", base.HEX)       -- 79b Params (has been known to be 80 including sub_cmd e.g. razermouse)
f.f_crc = ProtoField.uint8("razer.crc", "CRC", base.HEX)                                            -- 1b CRC checksum
f.f_end_marker = ProtoField.uint8("razer.end", "End", base.HEX)                                     -- 1b End marker


local f_usb_ep_num = Field.new("usb.endpoint_number.endpoint") -- should be 0  
local f_usb_ep_dir = Field.new("usb.endpoint_number.direction") -- 1 IN, 0 OUT
local f_data_len = Field.new("usb.data_len") -- should be 90
local f_urb_len = Field.new("usb.urb_len") -- should be 90

local function getstring(fi)
    local ok, val = pcall(tostring, fi)
    if not ok then val = "(unknown)" end
    return val
end

-- write32 doesn't have a response on the in endpoint, it tweaks decoding on the _out_ endpoint
local responses = {
    NOTSET = 1, READMEM32 = 2, GENERIC = 3, READDEBUG = 4,
    WRITEMEM32 = 5,
    TRACECOUNT = 6
}
    
local expected = responses.NOTSET

function razer_proto.dissector(buffer, pinfo, tree)
    

        
    --[[This was very helpful for working out the field names I could use with Field.new()
    local fields = { all_field_infos() }
    for ix, finfo in ipairs(fields) do
        print(string.format("ix=%d, finfo.name = %s, finfo.value=%s", ix, finfo.name, getstring(finfo)))
    end
    print("\n")--]]

    local data_length = f_data_len()
    local data_direction = f_usb_ep_dir()
    local urb_length = f_urb_len()


    if (data_length.value == 90 and urb_length.value == 90) then
        pinfo.cols["protocol"] = "Razer"

        local offset = 0
        local t_razer = tree:add(razer_proto, buffer())
        
        if (data_direction.value == 0) then
            pinfo.cols["info"]:append("Request")
        else
            pinfo.cols["info"]:append("Response")
        end
        
        -- Add REQ,RES byte
        t_razer:add(f.f_reqtype, buffer(offset, 1))
        offset = offset + 1
        -- Add ID
        t_razer:add(f.f_id, buffer(offset, 1))
        offset = offset + 1
        -- Add Reserved 1
        t_razer:add(f.f_reserved1, buffer(offset, 3))
        offset = offset + 3
        -- Add Number of parameters bytes
        local num_params = buffer(offset, 1):le_uint()
        t_razer:add(f.f_number_parameter_bytes, buffer(offset, 1)) -- specifid buffer here instead of num params so it highlights in wireshark
        offset = offset + 1
        -- Add Reserved 2
        t_razer:add(f.f_reserved2, buffer(offset, 1))
        offset = offset + 1
        
        -- Add Command
        t_razer:add(f.f_command, buffer(offset, 1))
        offset = offset + 1

        -- Add sub command
        t_razer:add(f.f_sub_command, buffer(offset, 1))
        offset = offset + 1
        
        -- Add command params
        t_razer:add(f.f_command_params, buffer(offset, num_params - 1))
        offset = offset + 79 -- skip to the end
        
        -- Add CRC
        t_razer:add(f.f_crc, buffer(offset, 1))
        offset = offset + 1
        
        -- Add end
        t_razer:add(f.f_end_marker, buffer(offset, 1))
        offset = offset + 1
    end
end

usb_table = DissectorTable.get("usb.control")
usb_table:add(0xffff, razer_proto)
