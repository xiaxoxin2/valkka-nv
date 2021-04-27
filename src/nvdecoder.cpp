/*
 * nvdecoder.cpp : Cuda accelerated decoder class for libValkka
 *  
 * Authors: Xiao Xoxin <xiaoxoxin@gmail.com>
 * 
 * This file is part of the Valkka Nvidia cuda bridge.
 *
 * (c) Copyright 2021 Xiao Xoxin
 * 
 *            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE 
 *   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION 
 * 
 *  0. You just DO WHAT THE FUCK YOU WANT TO. 
 * 
 */

/** 
 *  @file    nvdecoder.cpp
 *  @author  Xiao Xoxin
 *  @date    2021
 *  @version 1.0.0 
 *  
 *  @brief   Cuda accelerated decoder class for libValkka
 */ 

#include "nvdecoder.h"

/*
legal stuff: https://developer.nvidia.com/nvidia-video-codec-sdk-license-agreement


Several API:s

"video codec sdk" : 
https://developer.nvidia.com/nvidia-video-codec-Sdk

register as an nvidia dev & get access to the sdk code

it's also avail. here:

https://github.com/NVIDIA/video-sdk-samples

Another API..? "linux sdk" : 
https://docs.nvidia.com/drive/drive-os-5.2.0.0L/drive-os/DRIVE_OS_Linux_SDK_Development_Guide/baggage/group__video__decoder__api.html

for jetson nano:
https://docs.nvidia.com/jetson/l4t-multimedia/classNvVideoDecoder.html

they're related..?

https://docs.nvidia.com/drive/drive_os_5.1.6.1L/nvvib_docs/index.html#page/DRIVE_OS_Linux_SDK_Development_Guide/Architecture/archi_nvmedia.html


At runtime, the program seems to be using this:

/usr/lib/x86_64-linux-gnu/libnvcuvid.so.440.33.01

However, the video SDK comes with this:

Lib/linux/stubs/x86_64/libnvcuvid.so

So:

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/sampsa/cuda/video-sdk-samples/Samples/NvCodec/Lib/linux/stubs/x86_64

& create a link to libcuvid.so.1 therein

NOPES!

Make sure that program uses this:

libnvcuvid.so.1 => /usr/lib/x86_64-linux-gnu/libnvcuvid.so.1 (0x00007f2499d63000)

TODO: make this almost identical to stuff in NvDecoder.cpp
(same variable names,)

*/

// adopted from "video-sdk-samples/Samples/AppDecode/AppDec/AppDec.cpp"
simplelogger::Logger *logger = simplelogger::LoggerFactory::CreateConsoleLogger();
// because Utils/NvCodecUtils.h has a STUPID "extern simplelogger::Logger *logger;" dependency

// adopted from "video-sdk-samples/Samples/NvCodec/NvDecoder/NvDecoder.cpp"
#define CUDA_DRVAPI_CALL( call )                                                                                                 \
    do                                                                                                                           \
    {                                                                                                                            \
        CUresult err__ = call;                                                                                                   \
        if (err__ != CUDA_SUCCESS)                                                                                               \
        {                                                                                                                        \
            const char *szErrName = NULL;                                                                                        \
            cuGetErrorName(err__, &szErrName);                                                                                   \
            std::ostringstream errorLog;                                                                                         \
            errorLog << "CUDA driver API error " << szErrName ;                                                                  \
            throw NVDECException::makeNVDECException(errorLog.str(), err__, __FUNCTION__, __FILE__, __LINE__);                   \
        }                                                                                                                        \
    }                                                                                                                            \
    while (0)

// adopted from "video-sdk-samples/Samples/NvCodec/NvDecoder/NvDecoder.cpp"
static const char * GetVideoCodecString(cudaVideoCodec eCodec) {
    static struct {
        cudaVideoCodec eCodec;
        const char *name;
    } aCodecName [] = {
        { cudaVideoCodec_MPEG1,     "MPEG-1"       },
        { cudaVideoCodec_MPEG2,     "MPEG-2"       },
        { cudaVideoCodec_MPEG4,     "MPEG-4 (ASP)" },
        { cudaVideoCodec_VC1,       "VC-1/WMV"     },
        { cudaVideoCodec_H264,      "AVC/H.264"    },
        { cudaVideoCodec_JPEG,      "M-JPEG"       },
        { cudaVideoCodec_H264_SVC,  "H.264/SVC"    },
        { cudaVideoCodec_H264_MVC,  "H.264/MVC"    },
        { cudaVideoCodec_HEVC,      "H.265/HEVC"   },
        { cudaVideoCodec_VP8,       "VP8"          },
        { cudaVideoCodec_VP9,       "VP9"          },
        { cudaVideoCodec_NumCodecs, "Invalid"      },
        { cudaVideoCodec_YUV420,    "YUV  4:2:0"   },
        { cudaVideoCodec_YV12,      "YV12 4:2:0"   },
        { cudaVideoCodec_NV12,      "NV12 4:2:0"   },
        { cudaVideoCodec_YUYV,      "YUYV 4:2:2"   },
        { cudaVideoCodec_UYVY,      "UYVY 4:2:2"   },
    };

    if (eCodec >= 0 && eCodec <= cudaVideoCodec_NumCodecs) {
        return aCodecName[eCodec].name;
    }
    for (int i = cudaVideoCodec_NumCodecs + 1; i < sizeof(aCodecName) / sizeof(aCodecName[0]); i++) {
        if (eCodec == aCodecName[i].eCodec) {
            return aCodecName[eCodec].name;
        }
    }
    return "Unknown";
}

// adopted from "video-sdk-samples/Samples/NvCodec/NvDecoder/NvDecoder.cpp"
static const char * GetVideoChromaFormatString(cudaVideoChromaFormat eChromaFormat) {
    static struct {
        cudaVideoChromaFormat eChromaFormat;
        const char *name;
    } aChromaFormatName[] = {
        { cudaVideoChromaFormat_Monochrome, "YUV 400 (Monochrome)" },
        { cudaVideoChromaFormat_420,        "YUV 420"              },
        { cudaVideoChromaFormat_422,        "YUV 422"              },
        { cudaVideoChromaFormat_444,        "YUV 444"              },
    };

    if (eChromaFormat >= 0 && eChromaFormat < sizeof(aChromaFormatName) / sizeof(aChromaFormatName[0])) {
        return aChromaFormatName[eChromaFormat].name;
    }
    return "Unknown";
}

// adopted from "video-sdk-samples/Samples/NvCodec/NvDecoder/NvDecoder.cpp"
static unsigned long GetNumDecodeSurfaces(cudaVideoCodec eCodec, unsigned int nWidth, unsigned int nHeight) {
    if (eCodec == cudaVideoCodec_VP9) {
        return 12;
    }

    if (eCodec == cudaVideoCodec_H264 || eCodec == cudaVideoCodec_H264_SVC || eCodec == cudaVideoCodec_H264_MVC) {
        // assume worst-case of 20 decode surfaces for H264
        return 20;
    }

    if (eCodec == cudaVideoCodec_HEVC) {
        // ref HEVC spec: A.4.1 General tier and level limits
        // currently assuming level 6.2, 8Kx4K
        int MaxLumaPS = 35651584;
        int MaxDpbPicBuf = 6;
        int PicSizeInSamplesY = (int)(nWidth * nHeight);
        int MaxDpbSize;
        if (PicSizeInSamplesY <= (MaxLumaPS>>2))
            MaxDpbSize = MaxDpbPicBuf * 4;
        else if (PicSizeInSamplesY <= (MaxLumaPS>>1))
            MaxDpbSize = MaxDpbPicBuf * 2;
        else if (PicSizeInSamplesY <= ((3*MaxLumaPS)>>2))
            MaxDpbSize = (MaxDpbPicBuf * 4) / 3;
        else
            MaxDpbSize = MaxDpbPicBuf;
        return (std::min)(MaxDpbSize, 16) + 4;
    }

    return 8;
}


bool CUDA_CALL(CUresult res) {
    if (res!=CUDA_SUCCESS) {
        const char *szErrName = NULL;
        cuGetErrorName(res, &szErrName);
        const char *szErrStr = NULL;
        cuGetErrorString(res, &szErrStr);
        decoderlogger.log(LogLevel::fatal) << "CUDA ERROR <" << szErrName << "> : "
            << szErrStr << std::endl;
        return false;
    }
    return true;
}            

bool NVDecoder::CudaCall(CUresult res) {
    if (CUDA_CALL(res)) {
        return true;
    }
    this->active=false;
    return false;
}

/*
* Code in the following cpp class has been adapted from "video-sdk-samples/Samples/NvCodec/NvDecoder/NvDecoder.cpp"
*/
NVDecoder::NVDecoder(AVCodecID av_codec_id, int gpu_index, int n_buf) : Decoder(), 
    av_codec_id(av_codec_id), active(true), semaring(n_buf), 
    m_hParser(NULL), m_hDecoder(NULL), aux_plane(NULL), current_pitch(0), current_height(0),
    first_timestamp(0), n_slot_aux(0), subsession_index_aux(-1) {
    // ck definition: Utils/NvCodecUtils.h
    int i;
    for(i=0; i<=n_buf; i++) {
        out_frame_rb.push_back(new AVBitmapFrame());
    }

    //iGpu = 0;
    iGpu = gpu_index;

    // DEVICE
    //ck(cuInit(iGpu)); // so nvidia doesn't know it's own api? ;D  the argument should be = 0 always
    int nGpu = 0; // int
    
    ck(cuDeviceGetCount(&nGpu));
    if (iGpu < 0 || iGpu >= nGpu) {
        decoderlogger.log(LogLevel::fatal) << "GPU ordinal out of range. Should be within [" << 0 << ", " << nGpu - 1 << "]" << std::endl;
        this->active = false;
        return;
    }
    cuDevice = 0; // CUdevice
    ck(cuDeviceGet(&cuDevice, iGpu));
    ck(cuDeviceGetName(szDeviceName, sizeof(szDeviceName), cuDevice));
    decoderlogger.log(LogLevel::normal) << "GPU in use: " << szDeviceName << std::endl;

    // CONTEXT // there seems to be no harm in creating a context per decoder..
    m_cuContext = NULL; // CUcontext
    ck(cuCtxCreate(&m_cuContext, 0, cuDevice));
    // enacpsulation: context[device[device_num]]

    // this is somewhat useful: http://codeofrob.com/entries/decoding-h264-with-nvidia.html
    //
    CUVIDPARSERPARAMS videoParserParameters = {};
    videoParserParameters.CodecType = FFmpeg2NvCodecId(this->av_codec_id);
    
    videoParserParameters.ulMaxNumDecodeSurfaces = 1;
    // videoParserParameters.ulMaxDisplayDelay = bLowLatency ? 0 : 1;
    videoParserParameters.ulMaxDisplayDelay = 0;
    videoParserParameters.pUserData = this; // give as the first parameter to the callback
    // it complicates matters that there is no way to pass custom data
    // at each call to cuvidParseVideoData

    videoParserParameters.pfnSequenceCallback = NVDecoder__sequenceCallback;
    videoParserParameters.pfnDecodePicture = NVDecoder__decodePicture;
    videoParserParameters.pfnDisplayPicture = NVDecoder__displayPicture;
    // cuCtxPushCurrent(m_cuContext);
    this->active = CUDA_CALL(cuvidCreateVideoParser(&m_hParser, &videoParserParameters));
}


NVDecoder::~NVDecoder() {
    //delete this->nv_dec;
    std::unique_lock<std::mutex> lk(mutex);
    if (m_hDecoder) {
        cuvidDestroyDecoder(m_hDecoder);
    }
    if (m_hParser) {
        cuvidDestroyVideoParser(m_hParser);
    }
    for (auto it=out_frame_rb.begin(); it!=out_frame_rb.end(); ++it) {
        delete *it;
    }
    if (aux_plane) {
        free(aux_plane);
    }
}

void NVDecoder::deactivate(const char* err) {
    decoderlogger.log(LogLevel::fatal) << "CUDA ERROR:" << err << std::endl;
    this->active = false;
    //TODO: use this & return of necessary & always check activate before going further
}


bool NVDecoder::isOk() {
    return this->active;
}

int NVDecoder::ReconfigureDecoder(CUVIDEOFORMAT *pVideoFormat)
{
    if (!active) {return -1;}
    if (pVideoFormat->bit_depth_luma_minus8 != m_videoFormat.bit_depth_luma_minus8 || pVideoFormat->bit_depth_chroma_minus8 != m_videoFormat.bit_depth_chroma_minus8){
        // NVDEC_THROW_ERROR("Reconfigure Not supported for bit depth change", CUDA_ERROR_NOT_SUPPORTED);
        deactivate("Reconfigure Not supported for bit depth change"); return -1;
    }

    if (pVideoFormat->chroma_format != m_videoFormat.chroma_format) {
        // NVDEC_THROW_ERROR("Reconfigure Not supported for chroma format change", CUDA_ERROR_NOT_SUPPORTED);
        deactivate("Reconfigure Not supported for chroma format change"); return -1;
    }

    bool bDecodeResChange = !(pVideoFormat->coded_width == m_videoFormat.coded_width && pVideoFormat->coded_height == m_videoFormat.coded_height);
    bool bDisplayRectChange = !(pVideoFormat->display_area.bottom == m_videoFormat.display_area.bottom && pVideoFormat->display_area.top == m_videoFormat.display_area.top \
        && pVideoFormat->display_area.left == m_videoFormat.display_area.left && pVideoFormat->display_area.right == m_videoFormat.display_area.right);

    int nDecodeSurface = GetNumDecodeSurfaces(pVideoFormat->codec, pVideoFormat->coded_width, pVideoFormat->coded_height);

    if ((pVideoFormat->coded_width > m_nMaxWidth) || (pVideoFormat->coded_height > m_nMaxHeight)) {
        // For VP9, let driver  handle the change if new width/height > maxwidth/maxheight
        if ((m_eCodec != cudaVideoCodec_VP9) || m_bReconfigExternal)
        {
            // NVDEC_THROW_ERROR("Reconfigure Not supported when width/height > maxwidth/maxheight", CUDA_ERROR_NOT_SUPPORTED);
            deactivate("Reconfigure Not supported when width/height > maxwidth/maxheight"); return -1;
        }
        return 1;
    }

    if (!bDecodeResChange && !m_bReconfigExtPPChange) {
        // if the coded_width/coded_height hasn't changed but display resolution has changed, then need to update width/height for 
        // correct output without cropping. Example : 1920x1080 vs 1920x1088 
        if (bDisplayRectChange)
        {
            m_nWidth = pVideoFormat->display_area.right - pVideoFormat->display_area.left;
            m_nHeight = pVideoFormat->display_area.bottom - pVideoFormat->display_area.top;
        }
        // no need for reconfigureDecoder(). Just return
        return 1;
    }

    CUVIDRECONFIGUREDECODERINFO reconfigParams = { 0 };

    reconfigParams.ulWidth = m_videoFormat.coded_width = pVideoFormat->coded_width;
    reconfigParams.ulHeight = m_videoFormat.coded_height = pVideoFormat->coded_height;

    // Dont change display rect and get scaled output from m_hDecoder. This will help display app to present apps smoothly
    reconfigParams.display_area.bottom = m_displayRect.b;
    reconfigParams.display_area.top = m_displayRect.t;
    reconfigParams.display_area.left = m_displayRect.l;
    reconfigParams.display_area.right = m_displayRect.r;
    reconfigParams.ulTargetWidth = m_nSurfaceWidth;
    reconfigParams.ulTargetHeight = m_nSurfaceHeight;

    // If external reconfigure is called along with resolution change even if post processing params is not changed,
    // do full reconfigure params update
    if ((m_bReconfigExternal && bDecodeResChange) || m_bReconfigExtPPChange) {
        // update display rect and target resolution if requested explicitely
        m_bReconfigExternal = false;
        m_bReconfigExtPPChange = false;
        m_videoFormat = *pVideoFormat;
        if (!(m_cropRect.r && m_cropRect.b) && !(m_resizeDim.w && m_resizeDim.h)) {
            m_nWidth = pVideoFormat->display_area.right - pVideoFormat->display_area.left;
            m_nHeight = pVideoFormat->display_area.bottom - pVideoFormat->display_area.top;
            reconfigParams.ulTargetWidth = pVideoFormat->coded_width;
            reconfigParams.ulTargetHeight = pVideoFormat->coded_height;
        }
        else {
            if (m_resizeDim.w && m_resizeDim.h) {
                reconfigParams.display_area.left = pVideoFormat->display_area.left;
                reconfigParams.display_area.top = pVideoFormat->display_area.top;
                reconfigParams.display_area.right = pVideoFormat->display_area.right;
                reconfigParams.display_area.bottom = pVideoFormat->display_area.bottom;
                m_nWidth = m_resizeDim.w;
                m_nHeight = m_resizeDim.h;
            }

            if (m_cropRect.r && m_cropRect.b) {
                reconfigParams.display_area.left = m_cropRect.l;
                reconfigParams.display_area.top = m_cropRect.t;
                reconfigParams.display_area.right = m_cropRect.r;
                reconfigParams.display_area.bottom = m_cropRect.b;
                m_nWidth = m_cropRect.r - m_cropRect.l;
                m_nHeight = m_cropRect.b - m_cropRect.t;
            }
            reconfigParams.ulTargetWidth = m_nWidth;
            reconfigParams.ulTargetHeight = m_nHeight;
        }

        m_nSurfaceHeight = reconfigParams.ulTargetHeight;
        m_nSurfaceWidth = reconfigParams.ulTargetWidth;
        m_displayRect.b = reconfigParams.display_area.bottom;
        m_displayRect.t = reconfigParams.display_area.top;
        m_displayRect.l = reconfigParams.display_area.left;
        m_displayRect.r = reconfigParams.display_area.right;
    }
    
    reconfigParams.ulNumDecodeSurfaces = nDecodeSurface;

    /*
    //START_TIMER
    CUDA_DRVAPI_CALL(cuCtxPushCurrent(m_cuContext));
    NVDEC_API_CALL(cuvidReconfigureDecoder(m_hDecoder, &reconfigParams));
    CUDA_DRVAPI_CALL(cuCtxPopCurrent(NULL));
    //STOP_TIMER("Session Reconfigure Time: ");
    */
    if (!CudaCall(cuCtxPushCurrent(m_cuContext))) {return -1;}
    if (!(CudaCall(cuvidReconfigureDecoder(m_hDecoder, &reconfigParams)))) {return -1;}
    if (!CudaCall(cuCtxPopCurrent(NULL))) {return -1;}
    return nDecodeSurface;
}


int CUDAAPI NVDecoder__sequenceCallback(void *obj, CUVIDEOFORMAT* pVideoFormat) {
    NVDecoder* nvdecoder = (NVDecoder*)(obj);
    nvdecoder->sequenceCallback(pVideoFormat);
}

int CUDAAPI NVDecoder__decodePicture(void* obj, CUVIDPICPARAMS* pPicParams) {
    NVDecoder* nvdecoder = (NVDecoder*)(obj);
    nvdecoder->decodePicture(pPicParams);
}

int CUDAAPI NVDecoder__displayPicture(void* obj, CUVIDPARSERDISPINFO* pDispInfo) {
    NVDecoder* nvdecoder = (NVDecoder*)(obj);
    nvdecoder->displayPicture(pDispInfo);
}



int NVDecoder::sequenceCallback(CUVIDEOFORMAT* pVideoFormat) {
    if (!active) {return -1;}
    // decoder initialization callback
    // std::cout << "sequenceCallback" << std::endl;
    // NVDecoder* me = (NVDecoder*)(obj);

    // CUVIDEOFORMAT:
    // Samples/NvCodec/NvDecoder/nvcuvid.h
    // CUVIDDECODECREATEINFO:
    // Samples/NvCodec/NvDecoder/cuviddec.h
    //
    CUresult cr;
    CUVIDDECODECREATEINFO decode_create_info = { 0 };

    decoderlogger.log(LogLevel::debug)
        << "Video Input Information" << std::endl
        << "\tCodec        : " << GetVideoCodecString(pVideoFormat->codec) << std::endl
        << "\tFrame rate   : " << pVideoFormat->frame_rate.numerator << "/" << pVideoFormat->frame_rate.denominator 
            << " = " << 1.0 * pVideoFormat->frame_rate.numerator / pVideoFormat->frame_rate.denominator << " fps" << std::endl
        << "\tSequence     : " << (pVideoFormat->progressive_sequence ? "Progressive" : "Interlaced") << std::endl
        << "\tCoded size   : [" << pVideoFormat->coded_width << ", " << pVideoFormat->coded_height << "]" << std::endl
        << "\tDisplay area : [" << pVideoFormat->display_area.left << ", " << pVideoFormat->display_area.top << ", " 
            << pVideoFormat->display_area.right << ", " << pVideoFormat->display_area.bottom << "]" << std::endl
        << "\tChroma       : " << GetVideoChromaFormatString(pVideoFormat->chroma_format) << std::endl
        << "\tBit depth    : " << pVideoFormat->bit_depth_luma_minus8 + 8
        << std::endl;

    int nDecodeSurface = GetNumDecodeSurfaces(
        pVideoFormat->codec, pVideoFormat->coded_width, pVideoFormat->coded_height);

    CUVIDDECODECAPS decodecaps;
    memset(&decodecaps, 0, sizeof(decodecaps));

    decodecaps.eCodecType = pVideoFormat->codec;
    decodecaps.eChromaFormat = pVideoFormat->chroma_format;
    decodecaps.nBitDepthMinus8 = pVideoFormat->bit_depth_luma_minus8; 

    CUDA_DRVAPI_CALL(cuCtxPushCurrent(m_cuContext));
    NVDEC_API_CALL(cuvidGetDecoderCaps(&decodecaps));
    CUDA_DRVAPI_CALL(cuCtxPopCurrent(NULL));
    
    if(!decodecaps.bIsSupported){
        // NVDEC_THROW_ERROR("Codec not supported on this GPU", CUDA_ERROR_NOT_SUPPORTED);
        // return nDecodeSurface;
        deactivate("Codec not supported on this GPU");
        return nDecodeSurface;
    }

    if ((pVideoFormat->coded_width > decodecaps.nMaxWidth) || 
        (pVideoFormat->coded_height > decodecaps.nMaxHeight)){
        //std::ostringstream errorString;
        //errorString << std::endl
        decoderlogger.log(LogLevel::fatal)
                    << "Resolution          : " << pVideoFormat->coded_width << "x" << pVideoFormat->coded_height << std::endl
                    << "Max Supported (wxh) : " << decodecaps.nMaxWidth << "x" << decodecaps.nMaxHeight << std::endl
                    << "Resolution not supported on this GPU";

        //const std::string cErr = errorString.str();
        //NVDEC_THROW_ERROR(cErr, CUDA_ERROR_NOT_SUPPORTED);
        this->active = false;
        return nDecodeSurface;
    }
    
    if ((pVideoFormat->coded_width>>4)*(pVideoFormat->coded_height>>4) > decodecaps.nMaxMBCount){
        //std::ostringstream errorString;
        //errorString << std::endl
        decoderlogger.log(LogLevel::fatal)
                    << "MBCount             : " << (pVideoFormat->coded_width >> 4)*(pVideoFormat->coded_height >> 4) << std::endl
                    << "Max Supported mbcnt : " << decodecaps.nMaxMBCount << std::endl
                    << "MBCount not supported on this GPU";
        //const std::string cErr = errorString.str();
        //NVDEC_THROW_ERROR(cErr, CUDA_ERROR_NOT_SUPPORTED);
        this->active = false;
        return nDecodeSurface;
    }
    
    if (m_nWidth && m_nHeight) {
        // cuvidCreateDecoder() has been called before, and now there's possible config change
        return ReconfigureDecoder(pVideoFormat);
    }

    // eCodec has been set in the constructor (for m_hParser). Here it's set again for potential correction
    m_eCodec = pVideoFormat->codec; // TODO: set in ctor
    m_eChromaFormat = pVideoFormat->chroma_format;
    m_nBitDepthMinus8 = pVideoFormat->bit_depth_luma_minus8;
    m_videoFormat = *pVideoFormat;

    CUVIDDECODECREATEINFO videoDecodeCreateInfo = { 0 };
    videoDecodeCreateInfo.CodecType = pVideoFormat->codec;
    videoDecodeCreateInfo.ChromaFormat = pVideoFormat->chroma_format;
    // videoDecodeCreateInfo.OutputFormat = pVideoFormat->bit_depth_luma_minus8 ? cudaVideoSurfaceFormat_P016 : cudaVideoSurfaceFormat_NV12;
    videoDecodeCreateInfo.OutputFormat = cudaVideoSurfaceFormat_NV12;
    videoDecodeCreateInfo.bitDepthMinus8 = pVideoFormat->bit_depth_luma_minus8;
    videoDecodeCreateInfo.DeinterlaceMode = cudaVideoDeinterlaceMode_Weave;
    // videoDecodeCreateInfo.DeinterlaceMode = cudaVideoDeinterlaceMode_Adaptive;
    videoDecodeCreateInfo.ulNumOutputSurfaces = 2;
    // With PreferCUVID, JPEG is still decoded by CUDA while video is decoded by NVDEC hardware
    videoDecodeCreateInfo.ulCreationFlags = cudaVideoCreate_PreferCUVID;
    videoDecodeCreateInfo.ulNumDecodeSurfaces = nDecodeSurface;
    // videoDecodeCreateInfo.vidLock = m_ctxLock;
    videoDecodeCreateInfo.vidLock = NULL;
    videoDecodeCreateInfo.ulWidth = pVideoFormat->coded_width;
    videoDecodeCreateInfo.ulHeight = pVideoFormat->coded_height;
    if (m_nMaxWidth < (int)pVideoFormat->coded_width)
        m_nMaxWidth = pVideoFormat->coded_width;
    if (m_nMaxHeight < (int)pVideoFormat->coded_height)
        m_nMaxHeight = pVideoFormat->coded_height;
    videoDecodeCreateInfo.ulMaxWidth = m_nMaxWidth;
    videoDecodeCreateInfo.ulMaxHeight = m_nMaxHeight;

    if (!(m_cropRect.r && m_cropRect.b) && !(m_resizeDim.w && m_resizeDim.h)) {
        m_nWidth = pVideoFormat->display_area.right - pVideoFormat->display_area.left;
        m_nHeight = pVideoFormat->display_area.bottom - pVideoFormat->display_area.top;
        videoDecodeCreateInfo.ulTargetWidth = pVideoFormat->coded_width;
        videoDecodeCreateInfo.ulTargetHeight = pVideoFormat->coded_height;
    } else {
        if (m_resizeDim.w && m_resizeDim.h) {
            videoDecodeCreateInfo.display_area.left = pVideoFormat->display_area.left;
            videoDecodeCreateInfo.display_area.top = pVideoFormat->display_area.top;
            videoDecodeCreateInfo.display_area.right = pVideoFormat->display_area.right;
            videoDecodeCreateInfo.display_area.bottom = pVideoFormat->display_area.bottom;
            m_nWidth = m_resizeDim.w;
            m_nHeight = m_resizeDim.h;
        }

        if (m_cropRect.r && m_cropRect.b) {
            videoDecodeCreateInfo.display_area.left = m_cropRect.l;
            videoDecodeCreateInfo.display_area.top = m_cropRect.t;
            videoDecodeCreateInfo.display_area.right = m_cropRect.r;
            videoDecodeCreateInfo.display_area.bottom = m_cropRect.b;
            m_nWidth = m_cropRect.r - m_cropRect.l;
            m_nHeight = m_cropRect.b - m_cropRect.t;
        }
        videoDecodeCreateInfo.ulTargetWidth = m_nWidth;
        videoDecodeCreateInfo.ulTargetHeight = m_nHeight;
    }
    m_nSurfaceHeight = videoDecodeCreateInfo.ulTargetHeight;
    m_nSurfaceWidth = videoDecodeCreateInfo.ulTargetWidth;
    m_displayRect.b = videoDecodeCreateInfo.display_area.bottom;
    m_displayRect.t = videoDecodeCreateInfo.display_area.top;
    m_displayRect.l = videoDecodeCreateInfo.display_area.left;
    m_displayRect.r = videoDecodeCreateInfo.display_area.right;

    //std::cout << "Video Decoding Params:" << std::endl
    decoderlogger.log(LogLevel::debug)
        << "\tNum Surfaces : " << videoDecodeCreateInfo.ulNumDecodeSurfaces << std::endl
        << "\tCrop         : [" << videoDecodeCreateInfo.display_area.left << ", " << videoDecodeCreateInfo.display_area.top << ", "
        << videoDecodeCreateInfo.display_area.right << ", " << videoDecodeCreateInfo.display_area.bottom << "]" << std::endl
        << "\tResize       : " << videoDecodeCreateInfo.ulTargetWidth << "x" << videoDecodeCreateInfo.ulTargetHeight << std::endl
        << "\tDeinterlace  : " << std::vector<const char *>{"Weave", "Bob", "Adaptive"}[videoDecodeCreateInfo.DeinterlaceMode] 
        << std::endl;
    //std::cout << std::endl;

    /*
    CUDA_DRVAPI_CALL(cuCtxPushCurrent(m_cuContext));
    NVDEC_API_CALL(cuvidCreateDecoder(&m_hDecoder, &videoDecodeCreateInfo));
    CUDA_DRVAPI_CALL(cuCtxPopCurrent(NULL));
    */
    if (!CudaCall(cuCtxPushCurrent(m_cuContext))) {return nDecodeSurface;}
    if (!CudaCall(cuvidCreateDecoder(&m_hDecoder, &videoDecodeCreateInfo))) {return nDecodeSurface;}
    if (!CudaCall(cuCtxPopCurrent(NULL))) {return nDecodeSurface;}

    // STOP_TIMER("Session Initialization Time: ");

    /*
    if (sws_ctx != NULL) {
        sws_freeContext(sws_ctx);
        av_frame_free(&aux_av_frame);
        av_free(aux_av_frame);
        sws_ctx = NULL; // implies that no yuv conversion is needed
    }
    // hmm.. NV12 => YUV420P is just byte-shuffling
    // interpolation really not needed..
    sws_ctx =sws_getContext(
        out_av_frame->width, out_av_frame->height, 
        new_pixel_format, 
        out_av_frame->width, out_av_frame->height, 
        AV_PIX_FMT_YUV420P, 
        // SWS_POINT, 
        SWS_FAST_BILINEAR,
        NULL, NULL, NULL);
    */

    //std::cout << "(re)config decoder: w, h: " << m_nWidth << " " << m_nHeight << std::endl;
    for (auto it=out_frame_rb.begin(); it!=out_frame_rb.end(); ++it) {
        (*it)->reserve(m_nWidth, m_nHeight);
    }
    return nDecodeSurface;
}

int NVDecoder::decodePicture(CUVIDPICPARAMS* pPicParams) {
    if (!active) {return -1;}
    // encoded frames callback
    //std::cout << "decodePicture" << std::endl;
    // NVDecoder* me = (NVDecoder*)(obj);
    CUresult cr;

    if (!m_hDecoder) 
    {
        //NVDEC_THROW_ERROR("Decoder not initialized.", CUDA_ERROR_NOT_INITIALIZED);
        deactivate("decodePicture: something went wrong: decoder not initialized");
        return -1;
    }

    /*
    cuCtxPushCurrent(m_cuContext);
    cr = cuvidDecodePicture(m_hDecoder, pPicParams);
    cuCtxPopCurrent(NULL);

    if ( cr == CUDA_SUCCESS ) {
      return 1;
    } else {
      return -1;
    }
    */

    //NVDEC_API_CALL(cuvidDecodePicture(m_hDecoder, pPicParams));
    if (!CudaCall(cuvidDecodePicture(m_hDecoder, pPicParams))) {
        return -1;
    }
   return 1;
}

int NVDecoder::displayPicture(CUVIDPARSERDISPINFO* pDispInfo) {
    // decoded frames callback
    //std::cout << "displayPicture" << std::endl;
    if (!active) {return -1;}

    CUVIDPROCPARAMS videoProcessingParameters = {};
    videoProcessingParameters.progressive_frame = pDispInfo->progressive_frame;
    videoProcessingParameters.second_field = pDispInfo->repeat_first_field + 1;
    videoProcessingParameters.top_field_first = pDispInfo->top_field_first;
    videoProcessingParameters.unpaired_field = pDispInfo->repeat_first_field < 0;
    videoProcessingParameters.output_stream = m_cuvidStream;

    CUdeviceptr dpSrcFrame = 0;
    unsigned int nSrcPitch = 0;
    NVDEC_API_CALL(cuvidMapVideoFrame(m_hDecoder, pDispInfo->picture_index, &dpSrcFrame,
        &nSrcPitch, &videoProcessingParameters));

    CUVIDGETDECODESTATUS DecodeStatus;
    memset(&DecodeStatus, 0, sizeof(DecodeStatus));
    CUresult result = cuvidGetDecodeStatus(m_hDecoder, pDispInfo->picture_index, &DecodeStatus);
    if (result == CUDA_SUCCESS && (DecodeStatus.decodeStatus == cuvidDecodeStatus_Error || DecodeStatus.decodeStatus == cuvidDecodeStatus_Error_Concealed))
    {
        //printf("Decode Error occurred for picture %d\n", m_nPicNumInDecodeOrder[pDispInfo->picture_index]);
        decoderlogger.log(LogLevel::debug) << "NVDecoder: displayPicture: Decode Error occurred" << std::endl;
    }

    {// PROTECTED
        std::unique_lock<std::mutex> lk(mutex);
        int ind = semaring.write();
        if (ind < 0) {
            decoderlogger.log(LogLevel::normal) << "NVDecoder: handlePictureDisplay: overflow!" << std::endl;
            return -1;
        }
        // std::cout << "NVDecoder: using out_frame " << ind << std::endl;
        AVBitmapFrame *f = (AVBitmapFrame*)(out_frame_rb[ind]);

        CUDA_DRVAPI_CALL(cuCtxPushCurrent(m_cuContext));
        CUDA_MEMCPY2D m = { 0 };

        if (current_pitch!=nSrcPitch || current_height!=m_nHeight) {
            //std::cout << "NVDecoder: dims changed" << std::endl;
            current_pitch=nSrcPitch;
            current_height=m_nHeight;
            if (aux_plane) {
                free(aux_plane);
                aux_plane = NULL;
            }
        }

        // int byte_width = m_nDeviceFramePitch ? m_nDeviceFramePitch : m_nWidth * (m_nBitDepthMinus8 ? 2 : 1);
        int byte_width = m_nWidth * (m_nBitDepthMinus8 ? 2 : 1); // TODO: assume 1
        int byte_height = m_nHeight;
        // .. those are image w, h (1920, 1080)

        if (!aux_plane) {
            aux_plane = (uint8_t*)malloc(nSrcPitch*byte_height);
        }
        
        //src
        m.srcMemoryType = CU_MEMORYTYPE_DEVICE;
        m.srcDevice = dpSrcFrame;
        m.srcPitch = nSrcPitch;

        /*
        //dst
        if (!dstFrame) { // testing..
            dstFrame = malloc(byte_width*byte_height);
        }
        */

        m.dstMemoryType = CU_MEMORYTYPE_HOST;
        // m.dstPitch = m_nDeviceFramePitch ? m_nDeviceFramePitch : m_nWidth * (m_nBitDepthMinus8 ? 2 : 1);
        
        // m.WidthInBytes = m_nWidth * (m_nBitDepthMinus8 ? 2 : 1);
        // m.Height = m_nHeight;
        m.WidthInBytes = byte_width;
        m.Height = byte_height;

        // std::cout << "NVDecoder: displayPicture: w, h, pitch: " << byte_width << " " << byte_height << " " << nSrcPitch << std::endl;

        // AVBitmapFrame *f
        // encapsulates ffmpeg API's AVFrame
        // so, now we need to copy the bytes in-place
        // f->av_frame->data[0], data[1], data[2]
        // copy the luma plane as is into correct plance
        // chroma uv planes need some byte-sifting, so they
        // are copied to an aux memory array first

        // luma: directly in-place
        m.srcDevice = dpSrcFrame;
        m.dstPitch = f->bmpars.y_linesize;
        m.dstHost = f->y_payload;
        if (!CudaCall(cuMemcpy2DAsync(&m, m_cuvidStream))) {return -1;}

        /*
        https://gist.github.com/Jim-Bar/3cbba684a71d1a9d468a6711a6eddbeb
        https://github.com/FNNDSC/gpu/blob/master/shared/inc/cuvid/cuviddec.h#L35
        nice reference:
        https://www.cs.cmu.edu/afs/cs/academic/class/15668-s11/www/cuda-doc/html/group__CUDA__MEM_g4acf155faeb969d9d21f5433d3d0f274.html#g4acf155faeb969d9d21f5433d3d0f274
        could do the byte-sifting on the GPU .. but it's not well suited for such tasks
        https://developer.nvidia.com/npp
        https://stackoverflow.com/questions/65121668/convert-nv12-to-bgr-by-nvidia-performance-primitives
        opencv stuff at cuda..
        https://stackoverflow.com/questions/46807238/how-can-i-obtain-the-yuv-components-from-the-gpu-device
        */

        // interleaved UV planes
        m.dstPitch = nSrcPitch; // NOTE: same source (device) and target (host) pitch
        m.Height = m_nHeight / 2;
        m.srcDevice = (CUdeviceptr)((uint8_t *)dpSrcFrame
            + m.srcPitch * m_nSurfaceHeight);
        m.dstHost = aux_plane;
        if (!CudaCall(cuMemcpy2DAsync(&m, m_cuvidStream))) {return -1;}
        if (!CudaCall(cuStreamSynchronize(m_cuvidStream))) {return -1;}
        if (!CudaCall(cuCtxPopCurrent(NULL))) {return -1;}

        // NV12 interleaved to YUV420 planar
        int i,j;
        for(i=0; i<m.Height; i++) {
            for(j=0; j<(m.WidthInBytes/2)-1; j++) {
                f->u_payload[i*(f->bmpars.u_linesize) + j] 
                    = aux_plane[i*m.dstPitch + j*2];
                f->v_payload[i*(f->bmpars.v_linesize) + j] 
                    = aux_plane[i*m.dstPitch + j*2 + 1];
            }
        }
        /*
        // https://ffmpeg.org/doxygen/3.4/pixfmt_8h.html
        // AV_PIX_FMT_NV12
        height = sws_scale(sws_ctx, 
            (const uint8_t * const*)aux_av_frame->data,  // srcSlice[]
            aux_av_frame->linesize, // srcStride
            0,  // srcSliceY
            f->av_frame.height,  // srcSliceH
            f->av_frame.data, // dst[] // written
            f->av_frame.linesize); // dstStride[] // written
        */

        // f->copyMetaFrom(&in_frame);
        f->n_slot = n_slot_aux;
        f->subsession_index = subsession_index_aux;
        /*
        std::cout << "pDispInfo->timestamp: " << pDispInfo->timestamp << " " 
            << pDispInfo->timestamp/10000 << std::endl;
        */

        f->mstimestamp=(pDispInfo->timestamp/10000)+first_timestamp; // "10Mhz clock"
        //std::cout << *f << std::endl;
        //std::cout << f->dumpPayload() << std::endl;
    } // PROTECTED
    if (!CudaCall(cuvidUnmapVideoFrame(m_hDecoder, dpSrcFrame))) {return -1;}
    return 1;
}


void NVDecoder::flush() {
    if (!active) {return;}
    std::unique_lock<std::mutex> lk(this->mutex);
    semaring.reset();
}


Frame* NVDecoder::output() {
    if (!active) {return NULL;}
    std::unique_lock<std::mutex> lk(this->mutex);
    int ind = semaring.getIndex();
    if (ind < 0) {
        return NULL;
    }
    #ifdef NVDECODER_VERBOSE
    std::cout << "NVDecoder: output: returning index " << ind << std::endl;
    #endif
    return out_frame_rb[ind];
}


void NVDecoder::releaseOutput() {
    if (!active) {return;}
    std::unique_lock<std::mutex> lk(this->mutex);
    int ind = semaring.read();
}


bool NVDecoder::pull() {
    if (!active) {return false;}
    #ifdef NVDECODER_VERBOSE
    std::cout << "NVDecoder: pull" << std::endl;
    #endif
    if (!m_hParser)
    {
        // NVDEC_THROW_ERROR("Parser not initialized.", CUDA_ERROR_NOT_INITIALIZED);
        deactivate("NVDecoder: m_hParser not initialized");
        return false;
    }

    uint32_t flags = 0;
    // m_nDecodedFrame = 0;
    CUVIDSOURCEDATAPACKET packet = {0};
    packet.payload = in_frame.payload.data();
    packet.payload_size = in_frame.payload.size();
    packet.flags = flags | CUVID_PKT_TIMESTAMP;

    if (first_timestamp==0) {
        first_timestamp=in_frame.mstimestamp;
    }
    packet.timestamp = (in_frame.mstimestamp-first_timestamp)*10000; // "10Mhz clock"

    #ifdef NVDECODER_VERBOSE    
    std::cout << "packet.timestamp: " << packet.timestamp << std::endl;
    #endif

    //if (!pData || nSize == 0) {
    //    packet.flags |= CUVID_PKT_ENDOFSTREAM;
    //}
    //m_cuvidStream = stream;
    //if (m_pMutex) m_pMutex->lock();

    {// a hack since we can't pass custom data to cuvidParseVideoData
        std::unique_lock<std::mutex> lk(this->mutex);
        n_slot_aux = in_frame.n_slot;
        subsession_index_aux = in_frame.subsession_index;
    }
    NVDEC_API_CALL(cuvidParseVideoData(m_hParser, &packet));
    //TODO: push stuff to the decoder from in_frame
    {
        // check if there is stuff in the ringbuffer
        std::unique_lock<std::mutex> lk(this->mutex);
        if (semaring.isEmpty()) {
            #ifdef NVDECODER_VERBOSE
            std::cout << "NVDecoder: pull returning false" << std::endl;
            #endif
            return false;
        }
        else {
            #ifdef NVDECODER_VERBOSE
            std::cout << "NVDecoder: pull returning true" << std::endl;
            #endif
            return true;
            // NVDecoder::output() should be called immediately after this
        }
    }
}


/*
decoderthread:
    populate in_frame
    ok = NVDecoder::pull 
    if ok:
        frame = NVDecoder::output
        # send frame downstream
        NVDecoder::releaseOutput
*/
