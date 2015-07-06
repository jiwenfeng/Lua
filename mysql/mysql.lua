--[[
for k, v in pairs(mysql) do
	print(k, v)
end]]

local mysql = mysql:New()


assert(mysql:connect("localhost", "root", "wenfeng", "test", 10))

--[[


function insert()
	local ret, res = mysql:insert("insert into user values(8, \"wenfeng\", 10)")
	if not ret then
		print(res)
	else
		print("affect rows:", ret)
	end
end
--]]

function query()
	local ret, res = mysql:select("select * from user")
	if not ret then
		print(res)
	else
		for _, tb in ipairs(res) do
			for k, v in pairs(tb) do
				print(k, v)
			end
			print("-------------")
		end
	end
end

function batch_insert()
	mysql:begin()
	for i = 2, 30 do
		local sql = string.format("insert into user values(%d, \"Test%d\", %d, \"GuangZhou China\")", i, i, i)
		local ret, err = mysql:insert(sql)
		if not ret then
			print(err)
			mysql:rollback()
			return
		end
	end
	mysql:commit()
end

--[[
function delete()
	local sql_str = string.format("delete from user where age = 0")
	local ret, res = mysql:delete(sql_str)
	if not ret then
		print(res)
	end
	print("success")
end
--delete()
--]]


--batch_insert()
query()

--[[
local tbs = mysql:tables()
for k, v in pairs(tbs) do
	print(k, v)
end]]
