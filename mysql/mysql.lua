local mysql = mysql:New()

assert(mysql:connect("localhost", "root", "wenfeng", "wenfeng", 10))
function insert()
	local ret, res = mysql:insert("insert into user values(8, \"wenfeng\", 10)")
	if not ret then
		print(res)
	else
		print("affect rows:", ret)
	end
end

function query()
	local ret, res = mysql:query("select * from user")
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
	math.randomseed(os.time())
	mysql:begin()
	for i = 21, 30 do
		local sql_str = string.format("insert into user values(%d, \"%s\", %d)", i, "Test".. i, math.random() % 20)
		local ret, res = mysql:insert(sql_str)
		if not ret then
			mysql:rollback()
			print(res)
			return
		end
	end
	local sql_str = string.format("insert into user values(%d, \"%s\", %d)", 20, "Test".. 20, math.random() % 20)
	local ret, res = mysql:insert(sql_str)
	if not ret then
		mysql:rollback()
		print(res)
		return
	end
	mysql:commit()
	print("insert success")
end

function delete()
	local sql_str = string.format("delete from user where age = 0")
	local ret, res = mysql:delete(sql_str)
	if not ret then
		print(res)
	end
	print("success")
end
delete()

query()
