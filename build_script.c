/*
File:   build_script.c
Author: Taylor Robbins
Date:   05\16\2026
Description: 
	** None
*/

#define PIG_BUILD_PRINT_SYS_CMDS 0
#include "pig_build.h"

#define DEBUG_BUILD 1
#define COPY_TO_BIN 1
#define ONLY_BUILD_NON_EXISTANT_SAMPLES 1

#if DEBUG_BUILD
#define IF_DEBUG(...)    __VA_ARGS__
#define IF_RELEASE(...)  //nothing
#else
#define IF_DEBUG(...)    //nothing
#define IF_RELEASE(...)  __VA_ARGS__
#endif

#if BUILDING_ON_WINDOWS
#define SHDC_BIN_PATH "..\\sokol_tools\\bin\\win32\\sokol-shdc.exe"
#elif BUILDING_ON_LINUX
#define SHDC_BIN_PATH "../sokol_tools/bin/linux/sokol-shdc"
#elif BUILDING_ON_OSX_ARM
#define SHDC_BIN_PATH  "../sokol_tools/bin/osx_arm64/sokol-shdc"
#elif BUILDING_ON_OSX_INTEL
#define SHDC_BIN_PATH  "../sokol_tools/bin/osx/sokol-shdc"
#else
#error build_script.c SHDC_BIN_PATH needs to be updated to support the current platform!
#endif

#define MAX_DEPENDENCIES  4
typedef struct SappExample SappExample;
struct SappExample
{
	const char* name;
	bool isCpp;
	bool hasShader;
	bool isComputeShader;
	const char* dependencies[MAX_DEPENDENCIES];
};

void DownloadSokolIfNeeded();
void DownloadDCImguiIfNeeded();
void DownloadNuklearIfNeeded();

int main(int argc, char* argv[])
{
	RecompileIfNeeded(nullptr);
	Str pigBuildFolder = StrLit(PIG_BUILD_ROOT);
	IF_WINDOWS(bool isMsvcInitialized = WasMsvcDevBatchRun());
	
	DownloadSokolIfNeeded();
	DownloadDCImguiIfNeeded();
	// DownloadNuklearIfNeeded(); //TODO: Re-enable me once we have nuklear fully compiling/linking
	
	#if COPY_TO_BIN
	if (!DoesFolderExist(StrLit("../bin"))) { mkdir("../bin", FOLDER_PERMISSIONS); }
	#endif
	
	CliArgs commonArgs = EMPTY;
	
	// +==============================+
	// |     MSVC Compiler Flags      |
	// +==============================+
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_FULL_FILE_PATHS);
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_NO_LOGO);
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_LANG_C, CL_LANG_VERSION, "clatest"); //Use latest C language spec features
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_LANG_C, CL_EXPERIMENTAL, "c11atomics"); //Enables _Atomic types
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_LANG_CPP, CL_LANG_VERSION, "c++20");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, ".");
	// AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/../sokol");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/../sokol/util");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/sapp");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/libs");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|imgui", CL_INCLUDE_DIR, "[ROOT]/dcimgui/src");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|imgui-docking", CL_INCLUDE_DIR, "[ROOT]/dcimgui/src-docking");
	AddTaggedArgNt(&commonArgs, T_MSVC_CL "|cimgui", CL_INCLUDE_DIR, "[ROOT]/dcimgui/src");
	IF_DEBUG(AddTaggedArg(&commonArgs, T_MSVC_CL, CL_DEBUG_INFO));
	AddTaggedArgNt(&commonArgs, T_MSVC_CL, CL_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "d" : "2");
	
	// +==============================+
	// |     Clang Compiler Flags     |
	// +==============================+
	AddTaggedArg(&commonArgs, T_CLANG, CLANG_FULL_FILE_PATHS);
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_DEFINE, DEBUG_BUILD ? "DEBUG_BUILD=1" : "DEBUG_BUILD=0");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_OPTIMIZATION_LEVEL, DEBUG_BUILD ? "0" : "2");
	IF_DEBUG(AddTaggedArg(&commonArgs, T_CLANG, CLANG_DEBUG_INFO_DEFAULT));
	AddTaggedArgNt(&commonArgs, T_CLANG T_LANG_C, CLANG_LANG_VERSION, "gnu2x");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LANG_CPP, CLANG_LANG_VERSION, "c++20");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, ".");
	// AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/sokol");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/sokol/util");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/sapp");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/libs");
	AddTaggedArgNt(&commonArgs, T_CLANG "|imgui", CLANG_INCLUDE_DIR, "[ROOT]/dcimgui/src");
	AddTaggedArgNt(&commonArgs, T_CLANG "|imgui-docking", CLANG_INCLUDE_DIR, "[ROOT]/dcimgui/src-docking");
	AddTaggedArgNt(&commonArgs, T_CLANG "|cimgui", CLANG_INCLUDE_DIR, "[ROOT]/dcimgui/src");
	AddTaggedArgNt(&commonArgs, T_CLANG, CLANG_OUTPUT_FILE, "sokol_triangle");
	
	// +==============================+
	// |     Linux Compiler Flags     |
	// +==============================+
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "m");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "GL");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "X11");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xi");
	AddTaggedArgNt(&commonArgs, T_CLANG T_LINUX, CLANG_SYSTEM_LIBRARY, "Xcursor");
	
	// +==============================+
	// |      OSX Compiler Flags      |
	// +==============================+
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_DEFINE, "TARGET_IS_OSX=1");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "Cocoa");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "QuartzCore");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "CoreFoundation");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "Metal");
	AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, CLANG_FRAMEWORK, "MetalKit");
	
	AddTaggedArgNt(&commonArgs, T_MSVC_CL T_WINDOWS, CL_DEFINE,    "SOKOL_D3D11"); //SOKOL_D3D11, SOKOL_GLCORE, SOKOL_WGPU, SOKOL_VULKAN
	AddTaggedArgNt(&commonArgs, T_CLANG   T_OSX,     CLANG_DEFINE, "SOKOL_METAL"); //SOKOL_METAL, SOKOL_GLCORE
	AddTaggedArgNt(&commonArgs, T_CLANG   T_LINUX,   CLANG_DEFINE, "SOKOL_GLCORE"); //SOKOL_GLCORE, SOKOL_GLES3, SOKOL_WGPU, SOKOL_VULKAN
	
	// +==============================+
	// |         Linker Flags         |
	// +==============================+
	AddTaggedArg(&commonArgs, T_MSVC_CL, CL_LINK);
	AddTaggedArg(&commonArgs, T_MSVC_CL_OR_LINK, LINK_DISABLE_INCREMENTAL);
	AddTaggedArg(&commonArgs, T_MSVC_LINK, LINK_NO_LOGO);
	
	// +--------------------------------------------------------------+
	// |                Compile sokol Implementations                 |
	// +--------------------------------------------------------------+
	Str sokolObjFile = BUILDING_ON_WINDOWS ? StrLit("sokol.obj") : StrLit("sokol.o");
	if (!DoesFileExist(sokolObjFile))
	{
		CliArgs args = EMPTY;
		AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, "-x [VAL]", "objective-c");
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/sokol/sokol.c");
		AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
		AddTaggedArg(&args, T_CLANG, CLANG_COMPILE);
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, sokolObjFile);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, sokolObjFile);
		AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "sokol.pdb");
		AddArgList(&args, &commonArgs);
		
		#if BUILDING_ON_WINDOWS
		{
			PrintLine("[Building %.*s for WINDOWS...]", StrPrint(sokolObjFile));
			InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), T_MSVC_CL T_WINDOWS T_LANG_C, &args, FormatStr("Failed to compile sokol.c into %.*s", StrPrint(sokolObjFile)));
			AssertFileExist(sokolObjFile, true);
			PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(sokolObjFile));
		}
		#elif BUILDING_ON_OSX
		{
			PrintLine("[Building %.*s for OSX...]", StrPrint(sokolObjFile));
			RunCliProgramAndExitOnFailure(StrLit(EXE_CLANG), T_CLANG T_OSX T_LANG_C, &args, FormatStr("Failed to compile sokol.c into %.*s", StrPrint(sokolObjFile)));
			AssertFileExist(sokolObjFile, true);
			PrintLine("[Successfully built %.*s for OSX!]", StrPrint(sokolObjFile));
		}
		#else
		#error The current platform is not supported yet!
		#endif
	}
	Str sokolDllFile = BUILDING_ON_WINDOWS ? StrLit("sokol.dll") : (BUILDING_ON_OSX ? StrLit("sokol.dylib") : StrLit("sokol.so"));
	Str sokolLibFile = BUILDING_ON_WINDOWS ? StrLit("sokol.lib") : (BUILDING_ON_OSX ? StrLit("sokol.dylib") : StrLit("sokol.so"));
	if (!DoesFileExist(sokolDllFile))
	{
		CliArgs args = EMPTY;
		AddTaggedArgNt(&commonArgs, T_CLANG T_OSX, "-x [VAL]", "objective-c");
		AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/sokol/sokol-dll.c");
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, sokolDllFile);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, sokolDllFile);
		AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "sokol_dll.pdb");
		AddArgList(&args, &commonArgs);
		AddTaggedArg(&args, T_MSVC_CL, LINK_BUILD_DLL);
		
		#if BUILDING_ON_WINDOWS
		{
			PrintLine("[Building %.*s for WINDOWS...]", StrPrint(sokolDllFile));
			InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
			RunCliProgramAndExitOnFailure(StrLit(EXE_MSVC_CL), T_MSVC_CL T_WINDOWS T_LANG_C T_LIBRARY, &args, FormatStr("Failed to compile sokol-dll.c into %.*s", StrPrint(sokolDllFile)));
			AssertFileExist(sokolDllFile, true);
			PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(sokolDllFile));
		}
		#elif BUILDING_ON_OSX
		{
			PrintLine("[Building %.*s for OSX...]", StrPrint(sokolDllFile));
			RunCliProgramAndExitOnFailure(StrLit(EXE_CLANG), T_CLANG T_OSX T_LANG_C T_LIBRARY, &args, FormatStr("Failed to compile sokol-dll.c into %.*s", StrPrint(sokolDllFile)));
			AssertFileExist(sokolDllFile, true);
			PrintLine("[Successfully built %.*s for OSX!]", StrPrint(sokolDllFile));
		}
		#else
		#error The current platform is not supported yet!
		#endif
	}
	#if COPY_TO_BIN
	CopyFileToFolder(sokolDllFile, StrLit("../bin"), true);
	#endif
	
	// +--------------------------------------------------------------+
	// |                        Compile Imgui                         |
	// +--------------------------------------------------------------+
	Str imguiLibPath = StrLit("imgui.lib");
	Str imguiDllPath = StrLit("imgui.dll");
	Str imguiDockingLibPath = StrLit("imgui_docking.lib");
	Str imguiDockingDllPath = StrLit("imgui_docking.dll");
	for (u64 dockingEnabled = 0; dockingEnabled < 2; dockingEnabled++)
	{
		Str libPath = (dockingEnabled ? imguiDockingLibPath : imguiLibPath);
		Str dllPath = (dockingEnabled ? imguiDockingDllPath : imguiDllPath);
		if (!DoesFileExist(dllPath) || !DoesFileExist(libPath))
		{
			PrintLine("[Building %.*s...]", StrPrint(dllPath));
			StrArray sourceFiles = EMPTY;
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/cimgui.cpp"          : "[ROOT]/dcimgui/src/cimgui.cpp");
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/cimgui_internal.cpp" : "[ROOT]/dcimgui/src/cimgui_internal.cpp");
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/imgui_demo.cpp"      : "[ROOT]/dcimgui/src/imgui_demo.cpp");
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/imgui_draw.cpp"      : "[ROOT]/dcimgui/src/imgui_draw.cpp");
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/imgui_tables.cpp"    : "[ROOT]/dcimgui/src/imgui_tables.cpp");
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/imgui_widgets.cpp"   : "[ROOT]/dcimgui/src/imgui_widgets.cpp");
			AddStrNt(&sourceFiles, dockingEnabled ? "[ROOT]/dcimgui/src-docking/imgui.cpp"           : "[ROOT]/dcimgui/src/imgui.cpp");
			
			// +==============================+
			// |   Build Imgui Source Files   |
			// +==============================+
			StrArray objFiles = EMPTY;
			for (u64 sIndex = 0; sIndex < sourceFiles.length; sIndex++)
			{
				Str sourcePath = sourceFiles.strings[sIndex];
				Str objPath = JoinStrings2(GetFileNamePart(sourcePath, false), BUILDING_ON_WINDOWS ? StrLit(".obj") : StrLit(".o"));
				
				CliArgs args = EMPTY;
				AddArgStr(&args, CLI_QUOTED_ARG, sourcePath);
				AddTaggedArg(&args, T_MSVC_CL, CL_COMPILE);
				AddTaggedArg(&args, T_CLANG, CLANG_COMPILE);
				AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, objPath);
				AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, objPath);
				AddTaggedArgNt(&args, T_MSVC_CL, CL_PDB_FILE, "imgui.pdb");
				IF_WINDOWS(AddTaggedArgNt(&args, T_MSVC_CL, CL_DEFINE, "CIMGUI_API=__declspec(dllexport)"));
				IF_WINDOWS(AddTaggedArgNt(&args, T_MSVC_CL, CL_DEFINE, "IMGUI_API=__declspec(dllexport)"));
				AddArgList(&args, &commonArgs);
				AddArgStr(&args, LINK_IMPORT_LIBRARY_FILE, libPath);
				
				StrArray tags = EMPTY;
				if (StrAnyCaseEquals(GetFileExtPart(sourcePath, false), StrLit(".c"))) { AddTag(&tags, T_LANG_C); }
				if (StrAnyCaseEquals(GetFileExtPart(sourcePath, false), StrLit(".cpp"))) { AddTag(&tags, T_LANG_CPP); }
				if (StrAnyCaseEquals(GetFileExtPart(sourcePath, false), StrLit(".cc"))) { AddTag(&tags, T_LANG_CPP); }
				
				#if BUILDING_ON_WINDOWS
				{
					AddTag(&tags, T_MSVC_CL);
					AddTag(&tags, T_MSVC_CL_OR_LINK);
					AddTag(&tags, T_WINDOWS);
					InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
					RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(sourcePath), StrPrint(objPath)));
					AssertFileExist(objPath, true);
				}
				#elif BUILDING_ON_OSX
				{
					AddTag(&tags, T_CLANG);
					AddTag(&tags, T_OSX);
					RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(sourcePath), StrPrint(objPath)));
					AssertFileExist(objPath, true);
				}
				#else
				#error The current platform is not supported yet!
				#endif
				
				AddStr(&objFiles, objPath);
			}
			
			// +==============================+
			// |        Link imgui.lib        |
			// +==============================+
			{
				CliArgs args = EMPTY;
				AddArg(&args, LINK_BUILD_DLL);
				for (u64 oIndex = 0; oIndex < objFiles.length; oIndex++)
				{
					AddArgStr(&args, CLI_QUOTED_ARG, objFiles.strings[oIndex]);
				}
				AddTaggedArgStr(&args, T_MSVC_LINK, LINK_OUTPUT_FILE, dllPath);
				AddTaggedArgStr(&args, T_MSVC_LINK, LINK_IMPORT_LIBRARY_FILE, libPath);
				AddArgList(&args, &commonArgs);
				
				StrArray tags = EMPTY;
				AddTag(&tags, T_LIBRARY);
				AddTag(&tags, "|imgui_lib");
				
				#if BUILDING_ON_WINDOWS
				{
					AddTag(&tags, T_MSVC_LINK);
					AddTag(&tags, T_MSVC_CL_OR_LINK);
					AddTag(&tags, T_WINDOWS);
					RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_LINK), &tags, &args, FormatStr("Failed to link %.*s", StrPrint(dllPath)));
					AssertFileExist(dllPath, true);
					AssertFileExist(libPath, true);
				}
				#elif BUILDING_ON_OSX
				{
					AddTag(&tags, T_CLANG);
					AddTag(&tags, T_OSX);
					RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &args, FormatStr("Failed to link %.*s", StrPrint(dllPath)));
					AssertFileExist(dllPath, true);
					AssertFileExist(libPath, true);
				}
				#else
				#error The current platform is not supported yet!
				#endif
			}
			
			PrintLine("[Successfully built %.*s!]", StrPrint(dllPath));
		}
	}
	#if COPY_TO_BIN
	CopyFileToFolder(imguiDllPath, StrLit("../bin"), true);
	CopyFileToFolder(imguiDockingDllPath, StrLit("../bin"), true);
	#endif
	
	// +--------------------------------------------------------------+
	// |                       Compile Examples                       |
	// +--------------------------------------------------------------+
	SappExample exampleDefs[] = {
		{ .name = "arraytex-sapp",              .hasShader=true },
		{ .name = "basisu-sapp",                .hasShader=false, .dependencies={ "basisu" } },
		{ .name = "blend-op-sapp",              .hasShader=true },
		{ .name = "blend-playground-sapp",      .hasShader=true, .dependencies={ "cimgui", "fileutil", "qoi" } },
		{ .name = "blend-sapp",                 .hasShader=true },
		{ .name = "bufferoffsets-sapp",         .hasShader=true },
		{ .name = "cgltf-sapp",                 .hasShader=true, .dependencies={ "basisu", "fileutil" } },
		{ .name = "cimgui-sapp",                .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "clear-sapp",                 .hasShader=false },
		{ .name = "computeboids-sapp",          .hasShader=true, .isComputeShader=true, .dependencies={ "cimgui" } },
		{ .name = "cube-sapp",                  .hasShader=true },
		// { .name = "cubemap-jpeg-sapp",          .hasShader=true }, //TODO: Depends on stb_image.h?
		{ .name = "cubemaprt-sapp",             .hasShader=true },
		{ .name = "cursor-sapp",                .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "customresolve-sapp",         .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "debugtext-context-sapp",     .hasShader=true },
		{ .name = "debugtext-layers-sapp",      .hasShader=false },
		{ .name = "debugtext-printf-sapp",      .hasShader=false },
		{ .name = "debugtext-sapp",             .hasShader=false },
		{ .name = "debugtext-userfont-sapp",    .hasShader=false },
		{ .name = "drawcallperf-sapp",          .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "drawex-sapp",                .hasShader=true },
		{ .name = "droptest-sapp",              .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "dyntex-sapp",                .hasShader=true },
		{ .name = "dyntex3d-sapp",              .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "events-sapp",                .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "fontstash-layers-sapp",      .hasShader=true, .dependencies={ "fileutil" } },
		{ .name = "fontstash-sapp",             .hasShader=false, .dependencies={ "fileutil" } },
		{ .name = "framebuffer-sapp",           .hasShader=false },
		{ .name = "icon-sapp",                  .hasShader=false },
		{ .name = "ilbm-sapp",                  .hasShader=false, .dependencies={ "cimgui", "ilbm", "fileutil" } },
		// { .name = "imageblur-sapp",             .hasShader=true, .isComputeShader=true, .dependencies={ "cimgui" } }, //TODO: Depends on stb_image.h?
		{ .name = "imgui-dock-sapp",            .hasShader=false, .isCpp=true, .dependencies={ "imgui-docking" } },
		{ .name = "imgui-highdpi-sapp",         .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "imgui-images-sapp",          .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "imgui-perf-sapp",            .hasShader=false, .dependencies={ "cimgui" } },
		{ .name = "imgui-sapp",                 .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .name = "imgui-usercallback-sapp",    .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "instancing-compute-sapp",    .hasShader=true, .isComputeShader=true },
		{ .name = "instancing-pull-sapp",       .hasShader=true },
		{ .name = "instancing-sapp",            .hasShader=true },
		{ .name = "layerrender-sapp",           .hasShader=true },
		{ .name = "letterbox-sapp",             .hasShader=false, .dependencies={ "cimgui" } },
		// { .name = "loadpng-sapp",               .hasShader=true }, //TODO: Depends on stb_image.h?
		{ .name = "mipmap-sapp",                .hasShader=true },
		{ .name = "miprender-sapp",             .hasShader=true },
		// { .name = "modplay-sapp",               .hasShader=false }, //TODO: Depends on modplug.h
		{ .name = "mrt-pixelformats-sapp",      .hasShader=true },
		{ .name = "mrt-sapp",                   .hasShader=true },
		{ .name = "noentry-dll-sapp",           .hasShader=true, .dependencies={ "sokol_dll" } },
		// { .name = "noentry-sapp",               .hasShader=true }, //TODO: Depends on noentry version of sokol_app.h
		{ .name = "noninterleaved-sapp",        .hasShader=true },
		// { .name = "nuklear-images-sapp",        .hasShader=false, .dependencies={ "nuklear" } }, //TODO: Finish support for nuklear
		// { .name = "nuklear-sapp",               .hasShader=false, .dependencies={ "nuklear" } }, //TODO: Finish support for nuklear
		{ .name = "offscreen-msaa-sapp",        .hasShader=true },
		{ .name = "offscreen-sapp",             .hasShader=true },
		// { .name = "ozz-anim-sapp",              .hasShader=false, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		// { .name = "ozz-skin-sapp",              .hasShader=true, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		// { .name = "ozz-storagebuffer-sapp",     .hasShader=true, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		{ .name = "pixelformats-sapp",          .hasShader=true, .dependencies={ "cimgui" } },
		{ .name = "plmpeg-sapp",                .hasShader=true, .dependencies={ "fileutil" } },
		{ .name = "primtypes-sapp",             .hasShader=true },
		{ .name = "quad-sapp",                  .hasShader=true },
		// { .name = "restart-sapp",               .hasShader=true }, //TODO: Depends on modplug.h
		{ .name = "saudio-sapp",                .hasShader=false },
		{ .name = "sbufoffset-sapp",            .hasShader=true, .isComputeShader=true },
		{ .name = "sbuftex-sapp",               .hasShader=true },
		{ .name = "sdf-sapp",                   .hasShader=true },
		{ .name = "sgl-context-sapp",           .hasShader=false },
		{ .name = "sgl-lines-sapp",             .hasShader=false },
		// { .name = "sgl-microui-sapp",           .hasShader=false }, //TODO: Depends on microui.h
		{ .name = "sgl-points-sapp",            .hasShader=false },
		{ .name = "sgl-sapp",                   .hasShader=false },
		{ .name = "shadows-depthtex-sapp",      .hasShader=true },
		{ .name = "shadows-sapp",               .hasShader=true },
		{ .name = "shapes-sapp",                .hasShader=true },
		{ .name = "shapes-transform-sapp",      .hasShader=true },
		{ .name = "shared-bindings-sapp",       .hasShader=true },
		// { .name = "shdfeatures-sapp",           .hasShader=true, .dependencies={ "cimgui" } }, //TODO: Need a .glsl.none.h file?
		// { .name = "spine-contexts-sapp",        .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-inspector-sapp",       .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-layers-sapp",          .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-simple-sapp",          .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-skinsets-sapp",        .hasShader=false }, //TODO: Depends on spine.h
		// { .name = "spine-switch-skinsets-sapp", .hasShader=false }, //TODO: Depends on spine.h
		{ .name = "tex3d-sapp",                 .hasShader=true },
		{ .name = "texcube-sapp",               .hasShader=true },
		{ .name = "texview-sapp",               .hasShader=true, .dependencies={ "cimgui", "basisu", "fileutil" } },
		{ .name = "triangle-bufferless-sapp",   .hasShader=true },
		{ .name = "triangle-sapp",              .hasShader=true },
		{ .name = "uniformtypes-sapp",          .hasShader=true },
		{ .name = "uvwrap-sapp",                .hasShader=true },
		{ .name = "vertexindexbuffer-sapp",     .hasShader=true },
		{ .name = "vertexpull-sapp",            .hasShader=true },
		{ .name = "vertextexture-sapp",         .hasShader=true, .isComputeShader=true },
		{ .name = "write-storageimage-sapp",    .hasShader=true, .isComputeShader=true },
	};
	
	for (u64 eIndex = 0; eIndex < ArrayCount(exampleDefs); eIndex++)
	{
		SappExample def = exampleDefs[eIndex];
		Str exampleName = MakeStrNt(def.name);
		Str exampleFileName = JoinStrings2(exampleName, def.isCpp ? StrLit(".cc") : StrLit(".c"));
		Str exampleSrcPath = JoinStrings2(StrLit("[ROOT]/sapp/"), exampleFileName);
		#if BUILDING_ON_WINDOWS
		Str exampleBinName = JoinStrings2(exampleName, StrLit(".exe"));
		#else
		Str exampleBinName = exampleName;
		#endif
		StrReplaceChars(exampleBinName, '-', '_');
		Str shaderFileName = JoinStrings2(exampleName, StrLit(".glsl"));
		Str shaderFilePath = JoinStrings2(StrLit("[ROOT]/sapp/"), shaderFileName);
		Str shaderHeaderPath = JoinStrings2(exampleName, StrLit(".glsl.h"));
		Str shaderSrcPath = JoinStrings2(exampleName, def.isCpp ? StrLit(".glsl.cc") : StrLit(".glsl.c"));
		
		bool dependsOnSokolDll = false;
		bool dependsOnImgui = false;
		bool dependsOnImguiDocking = false;
		bool dependsOnCImgui = false;
		bool dependsOnFileUtil = false;
		bool dependsOnQoi = false;
		bool dependsOnIlbm = false;
		bool dependsOnBasisu = false;
		bool dependsOnNuklear = false;
		for (u64 dIndex = 0; dIndex < ArrayCount(def.dependencies); dIndex++)
		{
			if (def.dependencies[dIndex] == nullptr) { continue; }
			if (strcmp(def.dependencies[dIndex], "sokol_dll") == 0) { dependsOnSokolDll = true; }
			if (strcmp(def.dependencies[dIndex], "imgui") == 0) { dependsOnImgui = true; }
			if (strcmp(def.dependencies[dIndex], "imgui-docking") == 0) { dependsOnImguiDocking = true; }
			if (strcmp(def.dependencies[dIndex], "cimgui") == 0) { dependsOnCImgui = true; }
			if (strcmp(def.dependencies[dIndex], "fileutil") == 0) { dependsOnFileUtil = true; }
			if (strcmp(def.dependencies[dIndex], "qoi") == 0) { dependsOnQoi = true; }
			if (strcmp(def.dependencies[dIndex], "ilbm") == 0) { dependsOnIlbm = true; }
			if (strcmp(def.dependencies[dIndex], "basisu") == 0) { dependsOnBasisu = true; }
			if (strcmp(def.dependencies[dIndex], "nuklear") == 0) { dependsOnNuklear = true; }
		}
		Assert(!dependsOnImgui || def.isCpp);
		Assert(!dependsOnImguiDocking || def.isCpp);
		Assert(!dependsOnCImgui || !def.isCpp);
		
		// +==============================+
		// |     Cross-Compile Shader     |
		// +==============================+
		if (def.hasShader)
		{
			AssertFileExist(StrReplace(shaderFilePath, StrLit("[ROOT]"), StrLit("..")), false);
			if (!DoesFileExist(shaderHeaderPath))
			{
				PrintLine("Cross-compiling %.*s using sokol-shdc...", StrPrint(shaderFileName));
				
				CliArgs shdcArgs = EMPTY;
				AddArgNt(&shdcArgs, SHDC_FORMAT, "sokol_impl");
				AddArgNt(&shdcArgs, SHDC_ERROR_FORMAT, "msvc");
				// AddArg(&shdcArgs, SHDC_REFLECTION);
				if (def.isComputeShader)
				{
					AddArgNt(&shdcArgs, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:metal_macos");
				}
				else
				{
					AddArgNt(&shdcArgs, SHDC_SHADER_LANGUAGES, "hlsl5:glsl430:glsl300es:metal_macos");
				}
				AddArgStr(&shdcArgs, SHDC_INPUT, shaderFilePath);
				AddArgStr(&shdcArgs, SHDC_OUTPUT, shaderHeaderPath);
				
				int exitCode = RunCliProgram(StrLit(SHDC_BIN_PATH), "", &shdcArgs);
				if (exitCode != 0)
				{
					PrintLine("Failed to cross-compile %.*s using sokol-shdc! Exit code: %d", StrPrint(shaderFileName), exitCode);
				}
				else
				{
					AssertFileExist(shaderHeaderPath, true);
					PrintLine("Successfully cross-compiled %.*s!", StrPrint(shaderFileName));
				}
			}
			
			if (!DoesFileExist(shaderSrcPath))
			{
				PrintLine("Creating %.*s", StrPrint(shaderSrcPath));
				Str cCode = FormatStr("\n"
					"#include \"sokol_gfx.h\"\n"
					"#include \"vecmath/vecmath.h\"\n"
					"#define SOKOL_SHDC_IMPL\n"
					"#include \"%.*s\"",
					StrPrint(shaderHeaderPath)
				);
				CreateAndWriteFile(shaderSrcPath, cCode, true);
			}
		}
		
		// +==============================+
		// |        Build Example         |
		// +==============================+
		CliArgs args = EMPTY;
		AddArgStr(&args, CLI_QUOTED_ARG, exampleSrcPath);
		if (dependsOnSokolDll) { AddArgStr(&args, CLI_QUOTED_ARG, sokolLibFile); }
		else { AddArgStr(&args, CLI_QUOTED_ARG, sokolObjFile); }
		if (def.hasShader) { AddArgStr(&args, CLI_QUOTED_ARG, shaderSrcPath); }
		if (dependsOnImgui || dependsOnCImgui) { AddArgStr(&args, CLI_QUOTED_ARG, imguiLibPath); }
		if (dependsOnImguiDocking) { AddArgStr(&args, CLI_QUOTED_ARG, imguiDockingLibPath); }
		if (dependsOnFileUtil) { AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/util/fileutil.c"); }
		if (dependsOnIlbm) { AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/ilbm/ilbm.c"); }
		if (dependsOnBasisu) { AddArgNt(&args, CLI_QUOTED_ARG, "[ROOT]/libs/basisu/sokol_basisu.cpp"); }
		if (dependsOnNuklear)
		{
			AddTaggedArgNt(&args, T_MSVC_CL, CL_INCLUDE_DIR, "[ROOT]/nuklear");
			AddTaggedArgNt(&args, T_CLANG, CLANG_INCLUDE_DIR, "[ROOT]/nuklear");
		}
		if (dependsOnQoi) { AddTaggedArgNt(&args, T_MSVC_CL, CL_DEFINE, "QOI_IMPLEMENTATION"); }
		AddTaggedArgStr(&args, T_MSVC_CL, CL_BINARY_FILE, exampleBinName);
		AddTaggedArgStr(&args, T_CLANG, CLANG_OUTPUT_FILE, exampleBinName);
		IF_WINDOWS(AddTaggedArgStr(&args, T_MSVC_CL, CL_PDB_FILE, JoinStrings2(GetFileNamePart(exampleSrcPath, false), StrLit(".pdb"))));
		AddArgList(&args, &commonArgs);
		
		#if ONLY_BUILD_NON_EXISTANT_SAMPLES
		if (!DoesFileExist(exampleBinName))
		#endif
		{
			StrArray tags = EMPTY;
			if (def.isCpp) { AddTag(&tags, T_LANG_CPP); }
			else { AddTag(&tags, T_LANG_C); }
			#if BUILDING_ON_WINDOWS
			{
				PrintLine("[Building %.*s for WINDOWS...]", StrPrint(exampleBinName));
				InitializeMsvcIf(pigBuildFolder, &isMsvcInitialized);
				AddTag(&tags, T_MSVC_CL);
				AddTag(&tags, T_MSVC_CL_OR_LINK);
				AddTag(&tags, T_WINDOWS);
				for (u64 dIndex = 0; dIndex < ArrayCount(def.dependencies); dIndex++)
				{
					if (def.dependencies[dIndex] != nullptr) { AddStrNt(&tags, def.dependencies[dIndex]); }
				}
				RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_MSVC_CL), &tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(exampleFileName), StrPrint(exampleBinName)));
				AssertFileExist(exampleBinName, true);
				#if COPY_TO_BIN
				CopyFileToFolder(exampleBinName, StrLit("../bin"), true);
				#endif
				PrintLine("[Successfully built %.*s for WINDOWS!]", StrPrint(exampleBinName));
			}
			#elif BUILDING_ON_OSX
			{
				PrintLine("[Building %.*s for OSX...]", StrPrint(exampleBinName));
				AddTag(&tags, T_CLANG);
				AddTag(&tags, T_OSX);
				for (u64 dIndex = 0; dIndex < ArrayCount(def.dependencies); dIndex++)
				{
					if (def.dependencies[dIndex] != nullptr) { AddStrNt(&tags, def.dependencies[dIndex]); }
				}
				RunCliProgramTagArrayAndExitOnFailure(StrLit(EXE_CLANG), &tags, &args, FormatStr("Failed to compile %.*s into %.*s", StrPrint(exampleFileName), StrPrint(exampleBinName)));
				AssertFileExist(exampleBinName, true);
				#if COPY_TO_BIN
				CopyFileToFolder(exampleBinName, StrLit("../bin"), true);
				#endif
				PrintLine("[Successfully built %.*s for OSX!]", StrPrint(exampleBinName));
			}
			#else
			#error The current platform is not supported yet!
			#endif
		}
	}
}

// +--------------------------------------------------------------+
// |                    Download Dependencies                     |
// +--------------------------------------------------------------+
void DownloadSokolIfNeeded()
{
	// https://github.com/floooh/sokol/commit/453c71214fbb55d782683d20ea7e6c07314e3e9b
	// Commit 453c712 from May 14th 2026 - "sokol_framebuffer.h: fix some comment typos"
	Str sokolUrl = StrLit("https://github.com/floooh/sokol/archive/453c71214fbb55d782683d20ea7e6c07314e3e9b.zip");
	Str sokolZipPath = StrLit("sokol_453c712.zip");
	Str sokolZipRootFolder = StrLit("sokol-453c71214fbb55d782683d20ea7e6c07314e3e9b");
	Str sokolFolderPath = StrLit("../sokol");
	if (!DoesFileExist(sokolZipPath) || !DoesFolderExist(sokolFolderPath))
	{
		PrintLine("Downloading Sokol from \"%.*s\"", StrPrint(sokolUrl));
		// if (DoesFolderExist(sokolFolderPath)) { MyRemoveDirectory(sokolFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolUrl,
			sokolZipPath,
			1573414, 0x5707750981F0E26C,
			sokolFolderPath,
			sokolZipRootFolder
		);
	}
	
	// https://github.com/floooh/sokol-tools-bin/commit/1a9a4e54090fec42c5d13169b638f09f25474953
	// Commit 1a9a4e5 from April 26th 2026 - "updated (88)"
	Str sokolToolsUrl = StrLit("https://github.com/floooh/sokol-tools-bin/archive/1a9a4e54090fec42c5d13169b638f09f25474953.zip");
	Str sokolToolsZipPath = StrLit("sokol_tools_1a9a4e5.zip");
	Str sokolToolsZipRootFolder = StrLit("sokol-tools-bin-1a9a4e54090fec42c5d13169b638f09f25474953");
	Str sokolToolsFolderPath = StrLit("../sokol_tools");
	if (!DoesFileExist(sokolToolsZipPath) || !DoesFolderExist(sokolToolsFolderPath))
	{
		PrintLine("Downloading Sokol Tools from \"%.*s\"", StrPrint(sokolToolsUrl));
		// if (DoesFolderExist(sokolToolsFolderPath)) { MyRemoveDirectory(sokolToolsFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			sokolToolsUrl,
			sokolToolsZipPath,
			18032476, 0xDF94D1D90D33715F,
			sokolToolsFolderPath,
			sokolToolsZipRootFolder
		);
		
		// Mark sokol-shdc as executable on Linux and OSX
		#if BUILDING_ON_OSX || BUILDING_ON_LINUX
		CliArgs chmodArgs = EMPTY;
		AddArg(&chmodArgs, "+x");
		AddArgNt(&chmodArgs, CLI_QUOTED_ARG, SHDC_BIN_PATH);
		RunCliProgramAndExitOnFailure(StrLit("chmod"), "", &chmodArgs, StrLit("Failed to make sokol-shdc executable!"));
		#endif
	}
}

void DownloadDCImguiIfNeeded()
{
	// https://github.com/floooh/dcimgui/commit/f276b9ce0d03004916441daa28504c686cd37ecc
	// Commit f276b9c from April 15th 2026 - "build.zig: remove zig 0.15 support"
	Str dcImguiUrl = StrLit("https://github.com/floooh/dcimgui/archive/f276b9ce0d03004916441daa28504c686cd37ecc.zip");
	Str dcImguiZipPath = StrLit("dcimgui_v1.92.7.zip");
	Str dcImguiZipRootFolder = StrLit("dcimgui-f276b9ce0d03004916441daa28504c686cd37ecc");
	Str dcImguiFolderPath = StrLit("../dcimgui");
	if (!DoesFileExist(dcImguiZipPath) || !DoesFolderExist(dcImguiFolderPath))
	{
		PrintLine("Downloading dcimgui from \"%.*s\"", StrPrint(dcImguiUrl));
		// if (DoesFolderExist(dcImguiFolderPath)) { MyRemoveDirectory(dcImguiFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			dcImguiUrl,
			dcImguiZipPath,
			2434694, 0xCB162595C1A06724,
			dcImguiFolderPath,
			dcImguiZipRootFolder
		);
	}
}

void DownloadNuklearIfNeeded()
{
	// https://github.com/Immediate-Mode-UI/Nuklear
	// v4.13.3 from May 4th 2026 - https://github.com/Immediate-Mode-UI/Nuklear/releases/tag/v4.13.3
	Str nuklearUrl = StrLit("https://github.com/Immediate-Mode-UI/Nuklear/archive/refs/tags/v4.13.3.zip");
	Str nuklearZipPath = StrLit("nuklear_v4.13.3.zip");
	Str nuklearZipRootFolder = StrLit("Nuklear-4.13.3");
	Str nuklearFolderPath = StrLit("../nuklear");
	if (!DoesFileExist(nuklearZipPath) || !DoesFolderExist(nuklearFolderPath))
	{
		PrintLine("Downloading nuklear from \"%.*s\"", StrPrint(nuklearUrl));
		// if (DoesFolderExist(nuklearFolderPath)) { MyRemoveDirectory(nuklearFolderPath, true); } //TODO: Enable me once MyRemoveDirectory is implemented for OSX/Linux!
		DownloadAndExtractArchive(
			nuklearUrl,
			nuklearZipPath,
			2452230, 0x43B1710B6B246AF2,
			nuklearFolderPath,
			nuklearZipRootFolder
		);
	}
}
