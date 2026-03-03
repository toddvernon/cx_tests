
########################################################################################
# cx_tests - Top-level Makefile for all test suites
#
########################################################################################

########################################################################################
# Figure out the OS
#
########################################################################################
UNAME_S := $(shell uname -s | tr '[A-Z]' '[a-z]' )

ifeq ($(UNAME_S), linux)
	UNAME_R := $(shell uname -r | tr '[A-Z]' '[a-z]' )
	ARCH := $(shell uname -m | tr '[A-Z]' '[a-z]' )
endif

#if this is OSX
ifeq ($(UNAME_S), darwin)
	ARCH := $(shell uname -m | tr '[A-Z]' '[a-z]' )
endif

#if this is SUNOS or SOLARIS
ifeq ($(UNAME_S),sunos)
    UNAME_R := $(shell uname -r)
endif

#if this is IRIX
ifeq ($(UNAME_S),irix)
	UNAME_R := $(shell uname -r)
endif


########################################################################################
# Build all test suites
#
########################################################################################

all:
	@echo "========================================"
	@echo "Building cx_tests for: $(UNAME_S)_$(ARCH)"
	@echo "========================================"

# cxb64 tests (all platforms)

	@if [ -d "./cxb64" ]; then \
		echo "Building cxb64 tests..."; \
		cd cxb64; make; \
	fi

# cxbuildoutput tests (all platforms)

	@if [ -d "./cxbuildoutput" ]; then \
		echo "Building cxbuildoutput tests..."; \
		cd cxbuildoutput; make; \
	fi

# cxcallback tests (all platforms)

	@if [ -d "./cxcallback" ]; then \
		echo "Building cxcallback tests..."; \
		cd cxcallback; make; \
	fi

# cxcommandcompleter tests (all platforms)

	@if [ -d "./cxcommandcompleter" ]; then \
		echo "Building cxcommandcompleter tests..."; \
		cd cxcommandcompleter; make; \
	fi

# cxdatetime tests (uses tz library - not on sunos or irix)

	@if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ "$(UNAME_S)" != "irix" ]; then \
			if [ -d "./cxdatetime" ]; then \
				echo "Building cxdatetime tests..."; \
				cd cxdatetime; make; \
			fi; \
		fi; \
	fi

# cxdirectory tests (all platforms)

	@if [ -d "./cxdirectory" ]; then \
		echo "Building cxdirectory tests..."; \
		cd cxdirectory; make; \
	fi

# cxeditbuffer tests (all platforms)

	@if [ -d "./cxeditbuffer" ]; then \
		echo "Building cxeditbuffer tests..."; \
		cd cxeditbuffer; make; \
	fi

# cxeditline tests (all platforms)

	@if [ -d "./cxeditline" ]; then \
		echo "Building cxeditline tests..."; \
		cd cxeditline; make; \
	fi

# cxexpression tests (all platforms)

	@if [ -d "./cxexpression" ]; then \
		echo "Building cxexpression tests..."; \
		cd cxexpression; make; \
	fi

# cxfile tests (all platforms)

	@if [ -d "./cxfile" ]; then \
		echo "Building cxfile tests..."; \
		cd cxfile; make; \
	fi

# cxfunctor tests (all platforms)

	@if [ -d "./cxfunctor" ]; then \
		echo "Building cxfunctor tests..."; \
		cd cxfunctor; make; \
	fi

# cxhandle tests (all platforms)

	@if [ -d "./cxhandle" ]; then \
		echo "Building cxhandle tests..."; \
		cd cxhandle; make; \
	fi

# cxhashmap tests (all platforms)

	@if [ -d "./cxhashmap" ]; then \
		echo "Building cxhashmap tests..."; \
		cd cxhashmap; make; \
	fi

# cxjson tests (all platforms)

	@if [ -d "./cxjson" ]; then \
		echo "Building cxjson tests..."; \
		cd cxjson; make; \
	fi

# cxjsonutf tests (all platforms)

	@if [ -d "./cxjsonutf" ]; then \
		echo "Building cxjsonutf tests..."; \
		cd cxjsonutf; make; \
	fi

# cxlog tests (all platforms)

	@if [ -d "./cxlog" ]; then \
		echo "Building cxlog tests..."; \
		cd cxlog; make; \
	fi

# cxlogfile tests (all platforms)

	@if [ -d "./cxlogfile" ]; then \
		echo "Building cxlogfile tests..."; \
		cd cxlogfile; make; \
	fi

# cxnet tests (all platforms)

	@if [ -d "./cxnet" ]; then \
		echo "Building cxnet tests..."; \
		cd cxnet; make; \
	fi

# cxprop tests (all platforms)

	@if [ -d "./cxprop" ]; then \
		echo "Building cxprop tests..."; \
		cd cxprop; make; \
	fi

# cxregex tests (darwin and linux only - regex lib not available on other platforms)

	@if [ "$(UNAME_S)" = "darwin" ] || [ "$(UNAME_S)" = "linux" ]; then \
		if [ -d "./cxregex" ]; then \
			echo "Building cxregex tests..."; \
			cd cxregex; make; \
		fi; \
	fi

# cxscreen tests (all platforms)

	@if [ -d "./cxscreen" ]; then \
		echo "Building cxscreen tests..."; \
		cd cxscreen; make; \
	fi

# cxslist tests (all platforms)

	@if [ -d "./cxslist" ]; then \
		echo "Building cxslist tests..."; \
		cd cxslist; make; \
	fi

# cxstar tests (all platforms)

	@if [ -d "./cxstar" ]; then \
		echo "Building cxstar tests..."; \
		cd cxstar; make; \
	fi

# cxstring tests (uses thread library - not on sunos)

	@if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ -d "./cxstring" ]; then \
			echo "Building cxstring tests..."; \
			cd cxstring; make; \
		fi; \
	fi

# cxthread tests (uses thread library - not on sunos)

	@if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ -d "./cxthread" ]; then \
			echo "Building cxthread tests..."; \
			cd cxthread; make; \
		fi; \
	fi

# cxtime tests (all platforms)

	@if [ -d "./cxtime" ]; then \
		echo "Building cxtime tests..."; \
		cd cxtime; make; \
	fi

# cxtz tests (uses tz library - not on sunos or irix)

	@if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ "$(UNAME_S)" != "irix" ]; then \
			if [ -d "./cxtz" ]; then \
				echo "Building cxtz tests..."; \
				cd cxtz; make; \
			fi; \
		fi; \
	fi

# cxutfcharacter tests (all platforms)

	@if [ -d "./cxutfcharacter" ]; then \
		echo "Building cxutfcharacter tests..."; \
		cd cxutfcharacter; make; \
	fi

# cxutfeditbuffer tests (all platforms)

	@if [ -d "./cxutfeditbuffer" ]; then \
		echo "Building cxutfeditbuffer tests..."; \
		cd cxutfeditbuffer; make; \
	fi

# cxutfstring tests (all platforms)

	@if [ -d "./cxutfstring" ]; then \
		echo "Building cxutfstring tests..."; \
		cd cxutfstring; make; \
	fi

# cxutfstringlist tests (all platforms)

	@if [ -d "./cxutfstringlist" ]; then \
		echo "Building cxutfstringlist tests..."; \
		cd cxutfstringlist; make; \
	fi

	@echo "========================================"
	@echo "All tests built for: $(UNAME_S)_$(ARCH)"
	@echo "========================================"


########################################################################################
# Run all tests
#
########################################################################################

test:
	@{ \
	echo "========================================"; \
	echo "Running cx_tests for: $(UNAME_S)"; \
	echo "========================================"; \
	\
	if [ -d "./cxb64" ]; then \
		echo ""; echo "=== cxb64 tests ==="; \
		(cd cxb64 && make test); \
	fi; \
	\
	if [ -d "./cxbuildoutput" ]; then \
		echo ""; echo "=== cxbuildoutput tests ==="; \
		(cd cxbuildoutput && make test); \
	fi; \
	\
	if [ -d "./cxcallback" ]; then \
		echo ""; echo "=== cxcallback tests ==="; \
		(cd cxcallback && make test); \
	fi; \
	\
	if [ -d "./cxcommandcompleter" ]; then \
		echo ""; echo "=== cxcommandcompleter tests ==="; \
		(cd cxcommandcompleter && make test); \
	fi; \
	\
	if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ "$(UNAME_S)" != "irix" ]; then \
			if [ -d "./cxdatetime" ]; then \
				echo ""; echo "=== cxdatetime tests ==="; \
				(cd cxdatetime && make test); \
			fi; \
		fi; \
	fi; \
	\
	if [ -d "./cxdirectory" ]; then \
		echo ""; echo "=== cxdirectory tests ==="; \
		(cd cxdirectory && make test); \
	fi; \
	\
	if [ -d "./cxeditbuffer" ]; then \
		echo ""; echo "=== cxeditbuffer tests ==="; \
		(cd cxeditbuffer && make test); \
	fi; \
	\
	if [ -d "./cxeditline" ]; then \
		echo ""; echo "=== cxeditline tests ==="; \
		(cd cxeditline && make test); \
	fi; \
	\
	if [ -d "./cxexpression" ]; then \
		echo ""; echo "=== cxexpression tests ==="; \
		(cd cxexpression && make test); \
	fi; \
	\
	if [ -d "./cxfile" ]; then \
		echo ""; echo "=== cxfile tests ==="; \
		(cd cxfile && make test); \
	fi; \
	\
	if [ -d "./cxfunctor" ]; then \
		echo ""; echo "=== cxfunctor tests ==="; \
		(cd cxfunctor && make test); \
	fi; \
	\
	if [ -d "./cxhandle" ]; then \
		echo ""; echo "=== cxhandle tests ==="; \
		(cd cxhandle && make test); \
	fi; \
	\
	if [ -d "./cxhashmap" ]; then \
		echo ""; echo "=== cxhashmap tests ==="; \
		(cd cxhashmap && make test); \
	fi; \
	\
	if [ -d "./cxjson" ]; then \
		echo ""; echo "=== cxjson tests ==="; \
		(cd cxjson && make test); \
	fi; \
	\
	if [ -d "./cxjsonutf" ]; then \
		echo ""; echo "=== cxjsonutf tests ==="; \
		(cd cxjsonutf && make test); \
	fi; \
	\
	if [ -d "./cxlog" ]; then \
		echo ""; echo "=== cxlog tests ==="; \
		(cd cxlog && make test); \
	fi; \
	\
	if [ -d "./cxlogfile" ]; then \
		echo ""; echo "=== cxlogfile tests ==="; \
		(cd cxlogfile && make test); \
	fi; \
	\
	if [ -d "./cxnet" ]; then \
		echo ""; echo "=== cxnet tests ==="; \
		(cd cxnet && make test); \
	fi; \
	\
	if [ -d "./cxprop" ]; then \
		echo ""; echo "=== cxprop tests ==="; \
		(cd cxprop && make test); \
	fi; \
	\
	if [ "$(UNAME_S)" = "darwin" ] || [ "$(UNAME_S)" = "linux" ]; then \
		if [ -d "./cxregex" ]; then \
			echo ""; echo "=== cxregex tests ==="; \
			(cd cxregex && make test); \
		fi; \
	fi; \
	\
	if [ -d "./cxscreen" ]; then \
		echo ""; echo "=== cxscreen tests ==="; \
		(cd cxscreen && make test); \
	fi; \
	\
	if [ -d "./cxslist" ]; then \
		echo ""; echo "=== cxslist tests ==="; \
		(cd cxslist && make test); \
	fi; \
	\
	if [ -d "./cxstar" ]; then \
		echo ""; echo "=== cxstar tests ==="; \
		(cd cxstar && make test); \
	fi; \
	\
	if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ -d "./cxstring" ]; then \
			echo ""; echo "=== cxstring tests ==="; \
			(cd cxstring && make test); \
		fi; \
	fi; \
	\
	if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ -d "./cxthread" ]; then \
			echo ""; echo "=== cxthread tests ==="; \
			(cd cxthread && make test); \
		fi; \
	fi; \
	\
	if [ -d "./cxtime" ]; then \
		echo ""; echo "=== cxtime tests ==="; \
		(cd cxtime && make test); \
	fi; \
	\
	if [ "$(UNAME_S)" != "sunos" ]; then \
		if [ "$(UNAME_S)" != "irix" ]; then \
			if [ -d "./cxtz" ]; then \
				echo ""; echo "=== cxtz tests ==="; \
				(cd cxtz && make test); \
			fi; \
		fi; \
	fi; \
	\
	if [ -d "./cxutfcharacter" ]; then \
		echo ""; echo "=== cxutfcharacter tests ==="; \
		(cd cxutfcharacter && make test); \
	fi; \
	\
	if [ -d "./cxutfeditbuffer" ]; then \
		echo ""; echo "=== cxutfeditbuffer tests ==="; \
		(cd cxutfeditbuffer && make test); \
	fi; \
	\
	if [ -d "./cxutfstring" ]; then \
		echo ""; echo "=== cxutfstring tests ==="; \
		(cd cxutfstring && make test); \
	fi; \
	\
	if [ -d "./cxutfstringlist" ]; then \
		echo ""; echo "=== cxutfstringlist tests ==="; \
		(cd cxutfstringlist && make test); \
	fi; \
	\
	echo ""; \
	echo "========================================"; \
	echo "Tests completed for: $(UNAME_S)"; \
	echo "(Check individual results above)"; \
	echo "========================================"; \
	} 2>&1 | tee TEST_RESULTS.txt


########################################################################################
# Clean up current platform specific object files
#
########################################################################################

clean:

	@if [ -d "./cxb64" ]; then \
		cd cxb64; make clean; \
	fi

	@if [ -d "./cxbuildoutput" ]; then \
		cd cxbuildoutput; make clean; \
	fi

	@if [ -d "./cxcallback" ]; then \
		cd cxcallback; make clean; \
	fi

	@if [ -d "./cxcommandcompleter" ]; then \
		cd cxcommandcompleter; make clean; \
	fi

	@if [ -d "./cxdatetime" ]; then \
		cd cxdatetime; make clean; \
	fi

	@if [ -d "./cxdirectory" ]; then \
		cd cxdirectory; make clean; \
	fi

	@if [ -d "./cxeditbuffer" ]; then \
		cd cxeditbuffer; make clean; \
	fi

	@if [ -d "./cxeditline" ]; then \
		cd cxeditline; make clean; \
	fi

	@if [ -d "./cxexpression" ]; then \
		cd cxexpression; make clean; \
	fi

	@if [ -d "./cxfile" ]; then \
		cd cxfile; make clean; \
	fi

	@if [ -d "./cxfunctor" ]; then \
		cd cxfunctor; make clean; \
	fi

	@if [ -d "./cxhandle" ]; then \
		cd cxhandle; make clean; \
	fi

	@if [ -d "./cxhashmap" ]; then \
		cd cxhashmap; make clean; \
	fi

	@if [ -d "./cxjson" ]; then \
		cd cxjson; make clean; \
	fi

	@if [ -d "./cxjsonutf" ]; then \
		cd cxjsonutf; make clean; \
	fi

	@if [ -d "./cxlog" ]; then \
		cd cxlog; make clean; \
	fi

	@if [ -d "./cxlogfile" ]; then \
		cd cxlogfile; make clean; \
	fi

	@if [ -d "./cxnet" ]; then \
		cd cxnet; make clean; \
	fi

	@if [ -d "./cxprop" ]; then \
		cd cxprop; make clean; \
	fi

	@if [ -d "./cxregex" ]; then \
		cd cxregex; make clean; \
	fi

	@if [ -d "./cxscreen" ]; then \
		cd cxscreen; make clean; \
	fi

	@if [ -d "./cxslist" ]; then \
		cd cxslist; make clean; \
	fi

	@if [ -d "./cxstar" ]; then \
		cd cxstar; make clean; \
	fi

	@if [ -d "./cxstring" ]; then \
		cd cxstring; make clean; \
	fi

	@if [ -d "./cxthread" ]; then \
		cd cxthread; make clean; \
	fi

	@if [ -d "./cxtime" ]; then \
		cd cxtime; make clean; \
	fi

	@if [ -d "./cxtz" ]; then \
		cd cxtz; make clean; \
	fi

	@if [ -d "./cxutfcharacter" ]; then \
		cd cxutfcharacter; make clean; \
	fi

	@if [ -d "./cxutfeditbuffer" ]; then \
		cd cxutfeditbuffer; make clean; \
	fi

	@if [ -d "./cxutfstring" ]; then \
		cd cxutfstring; make clean; \
	fi

	@if [ -d "./cxutfstringlist" ]; then \
		cd cxutfstringlist; make clean; \
	fi


########################################################################################
# Create tar archive for distribution
#
########################################################################################

archive:
	@echo "Creating cxtest_unix.tar..."
	@echo "  (extracts to cx_tests/ when untarred from parent directory)"
	@test -d ../ARCHIVE || mkdir ../ARCHIVE
	@tar cvf ../ARCHIVE/cxtest_unix.tar \
		--transform 's,^\./,cx_tests/,' \
		--exclude='*.o' \
		--exclude='*.a' \
		--exclude='.git' \
		--exclude='.claude' \
		--exclude='.DS_Store' \
		--exclude='TEST_RESULTS.txt' \
		--exclude='darwin_*' \
		--exclude='linux_*' \
		--exclude='sunos_*' \
		--exclude='irix_*' \
		--exclude='netbsd_*' \
		--exclude='nextstep_*' \
		--exclude='*.xcodeproj' \
		--exclude='*.xcworkspace' \
		--exclude='xcuserdata' \
		--exclude='DerivedData' \
		--exclude='*.pbxuser' \
		--exclude='*.mode1v3' \
		--exclude='*.mode2v3' \
		--exclude='*.perspectivev3' \
		--exclude='*.xcuserstate' \
		.
	@echo "Archive created: ../ARCHIVE/cxtest_unix.tar"

