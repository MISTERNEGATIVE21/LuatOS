/*
@module  httpsrv
@summary http服务端
@version 1.0
@date    2022.010.15
@demo wlan
@tag LUAT_USE_HTTPSRV
*/

#include "luat_base.h"
#include "luat_httpsrv.h"
#include "luat_network_adapter.h"
#include "luat_netdrv.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"

#define LUAT_LOG_TAG "httpsrv"
#include "luat_log.h"

/*
启动并监听一个http端口
@api httpsrv.start(port, func, adapter)
@int 端口号
@function 回调函数
@int 网络适配器编号, 默认是平台自带的网络协议栈
@return bool 成功返回true, 否则返回false
@usage

-- 监听80端口
httpsrv.start(80, function(client, method, uri, headers, body)
    -- method 是字符串, 例如 GET POST PUT DELETE
    -- uri 也是字符串 例如 / /api/abc
    -- headers table类型
    -- body 字符串
    log.info("httpsrv", method, uri, json.encode(headers), body)
    if uri == "/led/1" then
        LEDA(1)
        return 200, {}, "ok"
    elseif uri == "/led/0" then
        LEDA(0)
        return 200, {}, "ok"
    end
    -- 返回值的约定 code, headers, body
    -- 若没有返回值, 则默认 404, {} ,""
    return 404, {}, "Not Found" .. uri
end)
-- 关于静态文件
-- 情况1: / , 映射为 /index.html
-- 情况2: /abc.html , 先查找 /abc.html, 不存在的话查找 /abc.html.gz
-- 若gz存在, 会自动以压缩文件进行响应, 绝大部分浏览器支持.
-- 当前默认查找 /luadb/xxx 下的文件,暂不可配置
*/
static int l_httpsrv_start(lua_State *L) {
    luat_ip_addr_t local_ip, net_mask, gate_way, ipv6;
    char buff[64] = {0};
    int port = luaL_checkinteger(L, 1);
    if (!lua_isfunction(L, 2)) {
        LLOGW("httpsrv need callback function!!!");
        return 0;
    }
    luat_httpsrv_ctx_t ctx = {
        .port = port
    };
    uint8_t adapter_index = luaL_optinteger(L, 3, network_get_last_register_adapter());
    ctx.adapter_id = adapter_index;
    luat_netdrv_t* drv = luat_netdrv_get(adapter_index);
    if (drv == NULL || drv->netif == NULL) {
        LLOGW("该网络还没准备好 %d", adapter_index);
        return 0;
    }
    ctx.netif = drv->netif;
    lua_pushvalue(L, 2);
    ctx.lua_ref_id = luaL_ref(L, LUA_REGISTRYINDEX);
    int ret = luat_httpsrv_start(&ctx);
    if (ret == 0) {
        ipaddr_ntoa_r(&drv->netif->ip_addr, buff,  32);
        LLOGI("http listen at %s:%d", buff, ctx.port);
    }
    lua_pushboolean(L, ret == 0 ? 1 : 0);
    return 1;
}

/*
停止http服务
@api httpsrv.stop(port)
@int 端口号
@return nil 当前无返回值
*/
static int l_httpsrv_stop(lua_State *L) {
    int port = luaL_checkinteger(L, 1);
    luat_httpsrv_stop(port);
    return 0;
}

#include "rotable2.h"
static const rotable_Reg_t reg_httpsrv[] =
{
    {"start",        ROREG_FUNC(l_httpsrv_start) },
    {"stop",         ROREG_FUNC(l_httpsrv_stop) },
	{ NULL,          ROREG_INT(0) }
};

LUAMOD_API int luaopen_httpsrv( lua_State *L ) {
    luat_newlib2(L, reg_httpsrv);
    return 1;
}
