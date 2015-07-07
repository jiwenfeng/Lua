require "mysql"

local mysql = mysql:New();

--assert(mysql:connect("localhost", "abel", "0402"))
assert(mysql:connect("localhost", "abel", "0402", "jwf", 10, 10))

function query()
	local ret, res = mysql:execute("call sp_login_auths('wwww', 0, 12, 16777292, 16777291, 3)")
--	local ret, res = mysql:execute("call sp_load_all_user_smy")
--	local ret, res = mysql:execute("select * from user")
	if not ret then
		print(res)
		return
	end
	for _, tb in ipairs(res) do
		for k, v in pairs(tb) do
			print(k, v)
		end
		print('---------')
	end
end

function insert()
	for i = 1, 100 do
		local sql = string.format("insert into user values(%d, \"Test%d\", %d, \"GuangZhou GuangDong\")", i, i, i)
		local ret, res = mysql:execute(sql)
		if not ret then
			print(res)
			return
		end
	end
end

function update()
	local sql = "update user set addr = \"GuangZhou China\""
	local ret, res = mysql:execute(sql);
	if not ret then
		print(res)
		return
	end
	print("affect ", ret, "rows")
end

--insert()
query()
--update()
