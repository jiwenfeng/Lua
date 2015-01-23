local lib = require "lib"

function main()
	local tb = {1,2,3,4,5, a = 1, {b = 2, c = 3}, {d = 4, e = 5}}
	lib.loop(tb)
end
