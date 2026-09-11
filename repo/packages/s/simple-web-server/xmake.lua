package("simple-web-server")
    set_kind("library", {headeronly = true})
    set_homepage("https://gitlab.com/eidheim/Simple-Web-Server")
    set_description("A very simple, fast, multithreaded, platform independent HTTP and HTTPS server and client library.")
    set_license("MIT")

    add_urls("https://gitlab.com/eidheim/Simple-Web-Server.git")
    add_versions("2025.9.13", "546895a93a29062bb178367b46c7afb72da9881e")

    add_configs("standalone_asio", {description = "Use standalone Asio instead of Boost.Asio", default = false, type = "boolean"})
    add_configs("openssl", {description = "Use openssl for HTTPS support", default = true, type = "boolean"})

    add_deps("cmake")

    if is_plat("windows", "mingw") then
        add_syslinks("ws2_32", "wsock32")
    end
    if is_plat("linux", "bsd") then
        add_syslinks("pthread")
    end

    on_load(function (package)
        if package:config("openssl") then
            package:add("deps", "openssl3")
        end
        if package:config("standalone_asio") then
            package:add("deps", "asio")
        else
            package:add("deps", "boost", {configs = {filesystem = false, asio = true, system = true}})
        end
        package:add("includedirs", "include")
        package:add("includedirs", "include/simple-web-server")
    end)

    on_install("!wasm", function (package)
        io.replace("CMakeLists.txt", [[if(CMAKE_SOURCE_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")]], "if(FALSE)", {plain = true})
        io.replace("CMakeLists.txt", "install(", "endif()\nif(TRUE)\ninstall(", {plain = true})

        import("package.tools.cmake").install(package)
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <simple-web-server/server_http.hpp>
            void test() {
                SimpleWeb::Server<SimpleWeb::HTTP> server;
            }
        ]]}, {configs = {languages = "c++11"}}))
    end)