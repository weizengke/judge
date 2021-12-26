if (MSVC)
SET(SRCS_SECUREC memset_s)
else()
SET(SRCS_SECUREC
	fscanf_s
	fwscanf_s
	gets_s
	memcpy_s
    memmove_s
    memset_s
    scanf_s
    securecutil
    secureinput_a
    secureinput_w
    secureprintoutput_a
    secureprintoutput_w
    snprintf_s
    sprintf_s
    sscanf_s
    strcat_s
    strcpy_s
    strncat_s
    strncpy_s
    strtok_s
    swprintf_s
    swscanf_s
    vfscanf_s
    vsnprintf_s
    vsprintf_s
    vsscanf_s
    vswprintf_s
    vswscanf_s
    vwscanf_s
    wcscpy_s
    wcsncat_s
    wcsncpy_s
    wcstok_s
    wmemcpy_s
    wmemmove_s
    wscanf_s
	)
endif()

add_library(securec ${SRCS_SECUREC})