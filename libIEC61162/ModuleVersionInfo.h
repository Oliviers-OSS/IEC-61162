#ifndef _MODULE_VERSION_INFO_H_
#define _MODULE_VERSION_INFO_H_

#ifndef TO_STRING
#define STRING(x) #x
#define TO_STRING(x) STRING(x)
#endif

#define MODULE_NAME(name)						   \
static const char __module_name[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Name=" #name

#define MODULE_AUTHOR(name)						   \
static const char __module_author[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Author=" #name

#define MODULE_VERSION(modinfo)						   \
static const char __module_modinfo[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Version=" #modinfo

#define MODULE_FILE_VERSION(modinfo)						   \
static const char __module_file_modinfo[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"File modinfo=" #modinfo

#define MODULE_CLEARCASE_LABEL(label)						   \
static const char __module_cc_label[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Clearcase Label=" #label

#define MODULE_DESCRIPTION(description)						   \
static const char __module_description[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Description=" #description

#define MODULE_MANUFACTURER(manufacturer)						   \
static const char __module_description[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Manufacturer=" #manufacturer

#define MODULE_COPYRIGHT(copyright)						   \
static const char __module_copyright[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Copyright=" #copyright

#define MODULE_LANGUAGE(language)						   \
static const char __module_language[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Language=" #language

#define MODULE_NAME_AUTOTOOLS						   \
static const char __module_name[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Name=" PACKAGE_NAME

#define MODULE_VERSION_AUTOTOOLS						   \
static const char __module_modinfo[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Version=" PACKAGE_VERSION

#define MODULE_AUTHOR_AUTOTOOLS						   \
static const char __module_author[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Author=" PACKAGE_BUGREPORT

#define PACKAGE_NAME_AUTOTOOLS						   \
static const char __package_name[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Package=" PACKAGE_NAME

#define MODULE_LOGGER                                                  \
static const char __module_logger[] __attribute__((section(".modinfo"))) __attribute__((used)) = 	   \
"Logger=" TO_STRING(LOGGER)

#endif /* _MODULE_VERSION_INFO_H_ */
