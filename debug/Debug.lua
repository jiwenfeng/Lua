--'require "Debug" at the first line'

local DB = {state = 1, Breaks = {}, Help = {
	"[b file:line] Set a breakpoint at (line) in (file)",
	"[c] Continue running your program",
	"[n] Execute next program line",
	"[p var] Display the value of the (var)",
	"[bt] Display the program stack",
	"[list] List the code of where it is presently stopped",
	"[info] Display all breakpoints",
	"[enable file:line] Enable breakpoint",
	"[disable file:line] Disable breakpoint",
	"[delete file:line] Delete breakpoint",
	"[quit] Exit"
}}

local Function = 
{
	["help"] = function(content)
		for k, v in ipairs(DB.Help) do
			print(v)
		end
	end,
	["list"] = function(content)
		if DB.curFrame then
			local frame = DB.curFrame
			ShowSourceLine(frame.source, frame.currentline - 5, frame.currentline + 5)
		else
			print("No source code")
		end
		DB.state = 4
	end,
	["p"] = function(content)
		for i = 1, math.huge do
			local n, v = debug.getlocal(5, i)
			if not n then
				print(string.format("No symbol \"%s\" in current context", content))
				break
			end
			if(n == content) then
				PrintVariable(content, v)
				break
			end
		end
		DB.state = 4
	end,
	["b"] = function(content)
		local file, line = string.match(content, "%s*(.+)%s*:%s*(%d+)")
		if not file or not line then
			print("Syntax error. Try help")
			DB.state = 4
			return
		end
		local key = file .. ":" .. line
		if not DB.Breaks[key] then
			DB.Breaks[key] = true
		end
		DB.state = 4
		print(string.format("Set breakpoint at %s:%d", file, line))
	end,
	["c"] = function(content)
		DB.state = 1
	end,
	["n"] = function(content)
		DB.state = 2 
	end,
	["bt"] = function(content)
		for i = 0, math.huge do
			local info = debug.getinfo(5 + i, "nfSlLu")
			if not info then
				break
			end
			if info.what == "C" then
				print(string.format("[%s]:in ?", info.what))
			else
				print(string.format("%s:%d: in %s '%s'", info.short_src, info.currentline, info.namewhat, info.name or "main chunk"))
			end
		end
		DB.state = 4
	end,
	["info"] = function(content)
		for k, v in pairs(DB.Breaks) do
			print(k, v and "enable" or "disable")
		end
	end,
	["disable"] = function(content)
		DB.state = 4
		if content and DB.Breaks[content] then
			DB.Breaks[content] = false
			return
		end
		for k, v in pairs(DB.Breaks) do
			DB.Breaks[k] = false
		end
	end,
	["enable"] = function(content)
		DB.state = 4
		if content and DB.Breaks[content] == false then
			DB.Breaks[content] = true
			return
		end
		for k, v in pairs(DB.Breaks) do
			DB.Breaks[k] = true
		end
	end,
	["delete"] = function(content)
		DB.state = 4
		if content and DB.Breaks[content] ~= nil then
			DB.Breaks[content] = nil
			return
		end
		DB.Breaks = {}
	end,
	["where"] = function(content)
		if DB.curFrame then
			print(DB.curFrame.source, DB.cur.currentline)
		else
			print("No stack.")
		end
		DB.state = 4
	end,
	["quit"] = function(content)
		os.exit(0)
	end
}

function TableIsArray(tb)
	local n = 0
	for k, v in pairs(tb) do
		n = n + 1
	end
	return n == #tb
end

function FormatTable(tb)
	local str = ""
	local flag = false
	for k, v in pairs(tb) do
		if flag then
			str = str .. ","
		end
		flag = true
		if TableIsArray(tb) then
			if type(v) == "table" then
				str = str .. FormatTable(v)
			else
				str = str .. v
			end
		else
			if type(v) == "table" then
				str = str .. k .. " = " .. FormatTable(v)
			else
				str = str .. k .. " = " .. v
			end
		end
	end
	return "{" .. str .. "}"
end

function PrintVariable(name, value)
	if type(value) == "table" then
		local str = FormatTable(value)
		print(string.format("%s = %s", name, str))
	else
		print(string.format("%s = %s", name, value))
	end
end

function ProcessInput(input)
	local cmd, content = string.match(input, "%s*(%a+)%s*(.*)")
	if not Function[cmd] then
		print(string.format("Undefined command:\"%s\".Try \"help\"", cmd))
		DB.state = 4
		return
	end
	Function[cmd](content)
end

function ShowSourceLine(file, start_line, last_line, show_file)
	local prefix = string.sub(file, 1, 1)
	if prefix == "@" then
		file = string.sub(file, 2)
	end
	local handle = io.open(file, "r")
	if not handle then
		print("string.format(\"%s\" not exist)", file)
		return
	end
	local idx = 0
	for content in handle:lines() do
		idx = idx + 1
		if start_line <= idx and idx <= last_line then
			if show_file then
				print(string.format("%s:%d %s", file, idx, content))
			else
				print(string.format("%d %s", idx, content))
			end
			if(idx > last_line) then
				break
			end
		end
	end
	handle:close()
end

function PrintUsage()
	repeat 
		io.write("Debug>")
		local cmd = io.read()
		if not cmd then
			return
		end
		ProcessInput(cmd)
	until(DB.state ~= 4)
end

function Hook()
	if DB.state == 1 or DB.state == 4 then
		local info = debug.getinfo(2, "nfSlLu")
		if not DB.Breaks[info.short_src .. ":" .. info.currentline] then
			DB.curFrame = nil
			return
		end
		print(string.format("Hit breakpoint %s:%d", info.short_src, info.currentline))
		DB.curFrame = info
		PrintUsage()
		return
	end
	if DB.state == 2 then
		local info = debug.getinfo(2, "nfSlLu")
		if info.what == "C" then
			DB.curFrame = nil
			return
		end
		DB.curFrame = info
		ShowSourceLine(info.source, info.currentline, info.currentline, true)
		PrintUsage()
		return
	end
end

function Debug()
	PrintUsage()
	debug.sethook(Hook, "lcr")
end

Debug()
