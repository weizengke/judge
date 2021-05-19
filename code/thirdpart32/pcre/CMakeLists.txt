
SET(SRCS_PCRE 
	pcre_xclass
	pcre_version
	pcre_valid_utf8
	pcre_ucd
	pcre_try_flipped
	pcre_tables
	pcre_study
	pcre_refcount
	pcre_ord2utf8
	pcre_newline
	pcre_maketables
	pcre_info
	pcre_globals
	pcre_get
	pcre_fullinfo
	pcre_exec
	pcre_dfa_exec
	pcre_config
	pcre_compile
	pcre_chartables
	)

add_definitions("-DHAVE_CONFIG_H -DPCRE_STATIC")

add_library(pcre ${SRCS_PCRE})

