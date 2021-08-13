# snowflake算法C实现及lua绑定

```lua
local snowflake = require "snowflake"

snowflake.init(1)

local function gen_id()
	return snowflake.gen()
end

for i=1,10 do
	local id = gen_id()
	print("No: ",id)
end
```
