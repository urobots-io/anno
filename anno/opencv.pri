# OpenCV and OpenEXR support for the qmake / Qt Creator build.
#
# Windows: uses the install folders created by windows/build_dependencies.bat,
#   given by the environment variables OPENCV_DIR and OPENEXR_DIR,
#   or found below UR_LIB_DIR (opencv-<version>/install, openexr-<version>/install).
#   The OpenCV and OpenEXR DLLs are copied next to anno.exe after linking.
# Linux / macOS: uses OpenCV and OpenEXR installed in the system (pkg-config).
#
# Without OpenCV, anno is built with Qt image loading only (no EXR support).

ANNO_OPENCV_VERSION = 5.1.0
ANNO_OPENEXR_VERSION = 3.5.5
ANNO_IMATH_VERSION = 3.2.3

win32 {
    OPENCV_DIR = $$(OPENCV_DIR)
    OPENEXR_DIR = $$(OPENEXR_DIR)
    UR_LIB_DIR = $$(UR_LIB_DIR)
    !isEmpty(UR_LIB_DIR) {
        isEmpty(OPENCV_DIR): OPENCV_DIR = $$UR_LIB_DIR/opencv-$$ANNO_OPENCV_VERSION/install
        isEmpty(OPENEXR_DIR): OPENEXR_DIR = $$UR_LIB_DIR/openexr-$$ANNO_OPENEXR_VERSION/install
    }
    OPENCV_DIR = $$clean_path($$OPENCV_DIR)
    OPENEXR_DIR = $$clean_path($$OPENEXR_DIR)

    # OpenCV puts libraries into x64/vc<toolset>
    for (toolset, $$list(vc18 vc17 vc16)) {
        isEmpty(OPENCV_BIN_DIR):exists($$OPENCV_DIR/x64/$$toolset/lib) {
            OPENCV_LIB_DIR = $$OPENCV_DIR/x64/$$toolset/lib
            OPENCV_BIN_DIR = $$OPENCV_DIR/x64/$$toolset/bin
        }
    }

    !isEmpty(OPENCV_LIB_DIR):exists($$OPENEXR_DIR/include/OpenEXR) {
        ANNO_HAS_OPENCV = 1
        OPENCV_VER = $$replace(ANNO_OPENCV_VERSION, \\., )
        OPENEXR_SUFFIX = $$section(ANNO_OPENEXR_VERSION, ., 0, 0)_$$section(ANNO_OPENEXR_VERSION, ., 1, 1)
        IMATH_SUFFIX = $$section(ANNO_IMATH_VERSION, ., 0, 0)_$$section(ANNO_IMATH_VERSION, ., 1, 1)

        CONFIG(debug, debug|release) {
            CV_D = d
            EXR_D = _d
            OUT_SUBDIR = debug
        } else {
            OUT_SUBDIR = release
        }

        INCLUDEPATH += $$OPENCV_DIR/include \
            $$OPENEXR_DIR/include \
            $$OPENEXR_DIR/include/OpenEXR \
            $$OPENEXR_DIR/include/Imath
        LIBS += -L$$OPENCV_LIB_DIR \
            -lopencv_core$${OPENCV_VER}$${CV_D} \
            -lopencv_imgproc$${OPENCV_VER}$${CV_D} \
            -lopencv_imgcodecs$${OPENCV_VER}$${CV_D} \
            -lopencv_videoio$${OPENCV_VER}$${CV_D}
        LIBS += -L$$OPENEXR_DIR/lib \
            -lOpenEXR-$${OPENEXR_SUFFIX}$${EXR_D} \
            -lIex-$${OPENEXR_SUFFIX}$${EXR_D}
        DEFINES += OPENEXR_DLL IMATH_DLL

        # copy the DLLs next to anno.exe, so that it can be started from Qt Creator
        OUT_DIR = $$shell_path($$OUT_PWD/$$OUT_SUBDIR)
        QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote($$shell_path($$OPENCV_BIN_DIR/opencv_*$${OPENCV_VER}$${CV_D}.dll)) $$shell_quote($$OUT_DIR) $$escape_expand(\\n\\t)
        QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote($$shell_path($$OPENCV_BIN_DIR/opencv_videoio_ffmpeg$${OPENCV_VER}_64.dll)) $$shell_quote($$OUT_DIR) $$escape_expand(\\n\\t)
        EXR_DLLS = OpenEXR-$$OPENEXR_SUFFIX OpenEXRCore-$$OPENEXR_SUFFIX Iex-$$OPENEXR_SUFFIX IlmThread-$$OPENEXR_SUFFIX Imath-$$IMATH_SUFFIX
        for (dll, EXR_DLLS) {
            QMAKE_POST_LINK += $$QMAKE_COPY $$shell_quote($$shell_path($$OPENEXR_DIR/bin/$${dll}$${EXR_D}.dll)) $$shell_quote($$OUT_DIR) $$escape_expand(\\n\\t)
        }
    }
} else {
    CONFIG += link_pkgconfig
    packagesExist(opencv5) {
        PKGCONFIG += opencv5
        ANNO_HAS_OPENCV = 1
    } else:packagesExist(opencv4) {
        PKGCONFIG += opencv4
        ANNO_HAS_OPENCV = 1
    }
    !isEmpty(ANNO_HAS_OPENCV) {
        packagesExist(OpenEXR) {
            PKGCONFIG += OpenEXR
        } else {
            ANNO_HAS_OPENCV =
        }
    }
}

!isEmpty(ANNO_HAS_OPENCV) {
    DEFINES += ANNO_USE_OPENCV
    message("Building with OpenCV and OpenEXR")
} else {
    message("OpenCV / OpenEXR not found: building with Qt image loading only (no EXR support)")
}
