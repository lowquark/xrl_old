#!/usr/bin/lua

for i = 32,127 do
	print(string.format("#define XRL_KEY_%s\t0x%2X", string.char(i), i))
end
