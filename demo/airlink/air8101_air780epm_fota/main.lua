
-- LuaTools需要PROJECT和VERSION这两个信息
PROJECT = "fota2"
VERSION = "1.0.0"

-- 使用合宙iot平台时需要这个参数
PRODUCT_KEY = "1234" -- 到 iot.openluat.com 创建项目,获取正确的项目id

sys = require "sys"
libfota2 = require "libfota2"
sys.taskInit(function()
    -- sys.wait(500)
    airlink.init()
    airlink.start(0)
    log.info("创建桥接网络设备")
    netdrv.setup(socket.LWIP_USER0, netdrv.WHALE)
    netdrv.ipv4(socket.LWIP_USER0, "192.168.111.1", "255.255.255.0", "192.168.111.2")
end)

-- 循环打印版本号, 方便看版本号变化, 非必须
sys.taskInit(function()
    while 1 do
        sys.wait(2000)
        log.info("fota", "version", VERSION)
    end
end)

-- 升级结果的回调函数
local function fota_cb(ret)
    log.info("fota", ret)
    if ret == 0 then
        rtos.reboot()
    end
end

-- 使用合宙iot平台进行升级, 支持自定义参数, 也可以不配置
local ota_opts = {
    -- 合宙IOT平台的默认升级URL, 不填就是这个默认值
    -- 如果是自建的OTA服务器, 则需要填写正确的URL, 例如 http://192.168.1.5:8000/update
    -- 如果自建OTA服务器,且url包含全部参数,不需要额外添加参数, 请在url前面添加 ### 
    -- url="http://iot.openluat.com/api/site/firmware_upgrade",
    -- 请求的版本号, 合宙IOT有一套版本号体系,不传就是合宙规则, 自建服务器的话当然是自行约定版本号了
    -- version=_G.VERSION,
    -- 其他更多参数, 请查阅libfota2的文档 https://wiki.luatos.com/api/libs/libfota2.html
    url = "https://www.air32.cn/fota/air8101/adcdemo_1003.1.0_LuatOS-SoC_Air8101.bin?"
}
sys.taskInit(function()
    -- 这个判断是提醒要设置PRODUCT_KEY的,实际生产请删除
    if "123" == _G.PRODUCT_KEY and not ota_opts.url then
        while 1 do
            sys.wait(1000)
            log.info("fota", "请修改正确的PRODUCT_KEY")
        end
    end
    -- 等待网络就行后开始检查升级
    sys.waitUntil("net_ready",10000)
    sys.wait(500)
    libfota2.request(fota_cb, ota_opts)
end)

-- 演示定时自动升级, 每隔4小时自动检查一次
sys.timerLoopStart(libfota2.request, 4*3600000, fota_cb, ota_opts)

-- 用户代码已结束---------------------------------------------
-- 结尾总是这一句
sys.run()
-- sys.run()之后后面不要加任何语句!!!!!
