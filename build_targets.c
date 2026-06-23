/*
File:   build_targets.c
Author: Taylor Robbins
Date:   06\22\2026
Description: 
	** None
*/

#define MAX_DEPENDENCIES  4
typedef struct SampleDefinition SampleDefinition;
struct SampleDefinition
{
	const char* folder;
	const char* name;
	bool isCpp;
	bool hasShader;
	bool isComputeShader;
	bool noSokolObj;
	bool useSokolDll;
	const char* dependencies[MAX_DEPENDENCIES];
};

u64 GetSampleDefinitions(SampleDefinition* defsOutBuffer)
{
	SampleDefinition definitions[] = {
		// +--------------------------------------------------------------+
		// |                     sokol_app.h Examples                     |
		// +--------------------------------------------------------------+
		{ .folder="sapp", .name="arraytex-sapp",              .hasShader=true },
		{ .folder="sapp", .name="basisu-sapp",                .hasShader=false, .dependencies={ "basisu" } },
		{ .folder="sapp", .name="blend-op-sapp",              .hasShader=true },
		{ .folder="sapp", .name="blend-playground-sapp",      .hasShader=true, .dependencies={ "cimgui", "fileutil", "qoi" } },
		{ .folder="sapp", .name="blend-sapp",                 .hasShader=true },
		{ .folder="sapp", .name="bufferoffsets-sapp",         .hasShader=true },
		{ .folder="sapp", .name="cgltf-sapp",                 .hasShader=true, .dependencies={ "basisu", "fileutil" } },
		{ .folder="sapp", .name="cimgui-sapp",                .hasShader=false, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="clear-sapp",                 .hasShader=false },
		{ .folder="sapp", .name="computeboids-sapp",          .hasShader=true, .isComputeShader=true, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="cube-sapp",                  .hasShader=true },
		{ .folder="sapp", .name="cubemap-jpeg-sapp",          .hasShader=true, .dependencies={ "stb", "fileutil" } },
		{ .folder="sapp", .name="cubemaprt-sapp",             .hasShader=true },
		{ .folder="sapp", .name="cursor-sapp",                .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .folder="sapp", .name="customresolve-sapp",         .hasShader=true, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="debugtext-context-sapp",     .hasShader=true },
		{ .folder="sapp", .name="debugtext-layers-sapp",      .hasShader=false },
		{ .folder="sapp", .name="debugtext-printf-sapp",      .hasShader=false },
		{ .folder="sapp", .name="debugtext-sapp",             .hasShader=false },
		{ .folder="sapp", .name="debugtext-userfont-sapp",    .hasShader=false },
		{ .folder="sapp", .name="drawcallperf-sapp",          .hasShader=true, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="drawex-sapp",                .hasShader=true },
		{ .folder="sapp", .name="droptest-sapp",              .hasShader=false, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="dyntex-sapp",                .hasShader=true },
		{ .folder="sapp", .name="dyntex3d-sapp",              .hasShader=true, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="events-sapp",                .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .folder="sapp", .name="fontstash-layers-sapp",      .hasShader=true, .dependencies={ "fileutil" } },
		{ .folder="sapp", .name="fontstash-sapp",             .hasShader=false, .dependencies={ "fileutil" } },
		{ .folder="sapp", .name="framebuffer-sapp",           .hasShader=false },
		{ .folder="sapp", .name="icon-sapp",                  .hasShader=false },
		{ .folder="sapp", .name="ilbm-sapp",                  .hasShader=false, .dependencies={ "cimgui", "ilbm", "fileutil" } },
		{ .folder="sapp", .name="imageblur-sapp",             .hasShader=true, .isComputeShader=true, .dependencies={ "cimgui", "stb", "fileutil" } },
		{ .folder="sapp", .name="imgui-dock-sapp",            .hasShader=false, .isCpp=true, .dependencies={ "imgui-docking" } },
		{ .folder="sapp", .name="imgui-highdpi-sapp",         .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .folder="sapp", .name="imgui-images-sapp",          .hasShader=false, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="imgui-perf-sapp",            .hasShader=false, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="imgui-sapp",                 .hasShader=false, .isCpp=true, .dependencies={ "imgui" } },
		{ .folder="sapp", .name="imgui-usercallback-sapp",    .hasShader=true, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="instancing-compute-sapp",    .hasShader=true, .isComputeShader=true },
		{ .folder="sapp", .name="instancing-pull-sapp",       .hasShader=true },
		{ .folder="sapp", .name="instancing-sapp",            .hasShader=true },
		{ .folder="sapp", .name="layerrender-sapp",           .hasShader=true },
		{ .folder="sapp", .name="letterbox-sapp",             .hasShader=false, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="loadpng-sapp",               .hasShader=true, .dependencies={ "stb", "fileutil" } },
		{ .folder="sapp", .name="mipmap-sapp",                .hasShader=true },
		{ .folder="sapp", .name="miprender-sapp",             .hasShader=true },
		// { .folder="sapp", .name="modplay-sapp",               .hasShader=false }, //TODO: Depends on https://github.com/Konstanty/libmodplug (see https://github.com/floooh/fibs-libs/blob/main/libmodplug.ts)
		{ .folder="sapp", .name="mrt-pixelformats-sapp",      .hasShader=true },
		{ .folder="sapp", .name="mrt-sapp",                   .hasShader=true },
		{ .folder="sapp", .name="noentry-dll-sapp",           .hasShader=true, .useSokolDll=true },
		// { .folder="sapp", .name="noentry-sapp",               .hasShader=true }, //TODO: Depends on noentry version of sokol_app.h
		{ .folder="sapp", .name="noninterleaved-sapp",        .hasShader=true },
		{ .folder="sapp", .name="nuklear-images-sapp",        .hasShader=false, .dependencies={ "nuklear" } },
		{ .folder="sapp", .name="nuklear-sapp",               .hasShader=false, .dependencies={ "nuklear" } },
		{ .folder="sapp", .name="offscreen-msaa-sapp",        .hasShader=true },
		{ .folder="sapp", .name="offscreen-sapp",             .hasShader=true },
		// { .folder="sapp", .name="ozz-anim-sapp",              .hasShader=false, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		// { .folder="sapp", .name="ozz-skin-sapp",              .hasShader=true, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		// { .folder="sapp", .name="ozz-storagebuffer-sapp",     .hasShader=true, .isCpp=true, .dependencies={ "imgui" } }, //TODO: Cannot open include file: 'ozz/animation/runtime/animation.h': No such file or directory
		{ .folder="sapp", .name="pixelformats-sapp",          .hasShader=true, .dependencies={ "cimgui" } },
		{ .folder="sapp", .name="plmpeg-sapp",                .hasShader=true, .dependencies={ "fileutil" } },
		{ .folder="sapp", .name="primtypes-sapp",             .hasShader=true },
		{ .folder="sapp", .name="quad-sapp",                  .hasShader=true },
		// { .folder="sapp", .name="restart-sapp",               .hasShader=true }, //TODO: Depends on modplug.h
		{ .folder="sapp", .name="saudio-sapp",                .hasShader=false },
		{ .folder="sapp", .name="sbufoffset-sapp",            .hasShader=true, .isComputeShader=true },
		{ .folder="sapp", .name="sbuftex-sapp",               .hasShader=true },
		{ .folder="sapp", .name="sdf-sapp",                   .hasShader=true },
		{ .folder="sapp", .name="sgl-context-sapp",           .hasShader=false },
		{ .folder="sapp", .name="sgl-lines-sapp",             .hasShader=false },
		{ .folder="sapp", .name="sgl-microui-sapp",           .hasShader=false, .dependencies={ "microui" } },
		{ .folder="sapp", .name="sgl-points-sapp",            .hasShader=false },
		{ .folder="sapp", .name="sgl-sapp",                   .hasShader=false },
		{ .folder="sapp", .name="shadows-depthtex-sapp",      .hasShader=true },
		{ .folder="sapp", .name="shadows-sapp",               .hasShader=true },
		{ .folder="sapp", .name="shapes-sapp",                .hasShader=true },
		{ .folder="sapp", .name="shapes-transform-sapp",      .hasShader=true },
		{ .folder="sapp", .name="shared-bindings-sapp",       .hasShader=true },
		// { .folder="sapp", .name="shdfeatures-sapp",           .hasShader=true, .dependencies={ "cimgui" } }, //TODO: Need a .glsl.none.h file?
		// { .folder="sapp", .name="spine-contexts-sapp",        .hasShader=false }, //TODO: Depends on spine.h
		// { .folder="sapp", .name="spine-inspector-sapp",       .hasShader=false }, //TODO: Depends on spine.h
		// { .folder="sapp", .name="spine-layers-sapp",          .hasShader=false }, //TODO: Depends on spine.h
		// { .folder="sapp", .name="spine-simple-sapp",          .hasShader=false }, //TODO: Depends on spine.h
		// { .folder="sapp", .name="spine-skinsets-sapp",        .hasShader=false }, //TODO: Depends on spine.h
		// { .folder="sapp", .name="spine-switch-skinsets-sapp", .hasShader=false }, //TODO: Depends on spine.h
		{ .folder="sapp", .name="tex3d-sapp",                 .hasShader=true },
		{ .folder="sapp", .name="texcube-sapp",               .hasShader=true },
		{ .folder="sapp", .name="texview-sapp",               .hasShader=true, .dependencies={ "cimgui", "basisu", "fileutil" } },
		{ .folder="sapp", .name="triangle-bufferless-sapp",   .hasShader=true },
		{ .folder="sapp", .name="triangle-sapp",              .hasShader=true },
		{ .folder="sapp", .name="uniformtypes-sapp",          .hasShader=true },
		{ .folder="sapp", .name="uvwrap-sapp",                .hasShader=true },
		{ .folder="sapp", .name="vertexindexbuffer-sapp",     .hasShader=true },
		{ .folder="sapp", .name="vertexpull-sapp",            .hasShader=true },
		{ .folder="sapp", .name="vertextexture-sapp",         .hasShader=true, .isComputeShader=true },
		{ .folder="sapp", .name="write-storageimage-sapp",    .hasShader=true, .isComputeShader=true },
		
		// +--------------------------------------------------------------+
		// |                        GLFW Examples                         |
		// +--------------------------------------------------------------+
		{ .folder="glfw", .name="clear-glfw", .hasShader=false, .noSokolObj=true, .dependencies={ "glfw" } },
		// { .folder="glfw", .name="cube-glfw", .hasShader=false, .noSokolObj=true, .dependencies={ "glfw" } },
	};
	u64 numSamples = ArrayCount(definitions);
	if (defsOutBuffer != nullptr) { memcpy(defsOutBuffer, &definitions[0], sizeof(SampleDefinition) * numSamples); }
	return numSamples;
}
