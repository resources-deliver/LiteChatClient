cmake_minimum_required(VERSION 3.19)
project(LiteChatClient LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS 
	Core 
	Widgets
    Network
)

# 自动化启用C++标准要求、自动 MOC、UIC和 RCC
qt_standard_project_setup()

# 生成的可执行文件名称、GUI程序、链接源文件列表
# 此时cpp、h、ui文件链接的头文件路径无需更改，只需要将文件移入对应路径下
qt_add_executable(LiteChatClient
    # WIN32 MACOSX_BUNDLE  # qDebug在终端无效
    MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    src/networkmanager.cpp
    src/connectdialog.cpp
    src/usermanager.cpp
    src/logindialog.cpp
    src/registerdialog.cpp
    src/userinfodialog.cpp
    src/friendmanager.cpp
    src/searchresultdialog.cpp
    header/mainwindow.h
    header/networkmanager.h
    header/connectdialog.h
    header/usermanager.h
    header/logindialog.h
    header/registerdialog.h
    header/userinfodialog.h
    header/friendmanager.h
    header/searchresultdialog.h
    ui/mainwindow.ui
    ui/connectdialog.ui
    ui/logindialog.ui
    ui/registerdialog.ui
    ui/userinfodialog.ui
)

# 项目链接存放代码文件父层级（需要链接别的存放代码父层级，照例添加即可）
target_include_directories(LiteChatClient
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		${CMAKE_CURRENT_SOURCE_DIR}/header
		${CMAKE_CURRENT_SOURCE_DIR}/ui
)

# 项目链接UI文件层级(告诉AUTOUIC，ui文件全在./ui/)
set_target_properties(LiteChatClient PROPERTIES
	AUTOUIC_SEARCH_PATHS
		${CMAKE_CURRENT_SOURCE_DIR}/ui
)

# 项目链接Core、Widgets库（需要链接别的库就在后面添加即可）
target_link_libraries(LiteChatClient
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

if(MSVC)
    target_compile_options(LiteChatClient PRIVATE /utf-8)
endif()

# 安装规则
install(TARGETS LiteChatClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# 自动生成部署脚本并安装
qt_generate_deploy_app_script(
    TARGET LiteChatClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})