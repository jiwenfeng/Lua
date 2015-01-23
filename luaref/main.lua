
function test_func1()
	prin("this is test_func1")
end

function test_func2()
	print("this is test_func2")
end

function func2()
	printt("this is func2")
end

function func1()
	func2()
end


function test_func3(a)
	func1()
end


local class = {}

function class:New()
	local o = {}
	setmetatable(o, self);
	self.__index = self;
	return o
end

function class:Add(b)
	self.b = (self.b or 0) + b
end

function class:Get()
	return self.b or 0
end


TT(test_func1, test_func2, test_func3)
