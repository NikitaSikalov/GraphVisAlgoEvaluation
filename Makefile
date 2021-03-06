# Config
exe 		:= ./build/main
samples 	:= ./samples
results 	:= ./results
report_file := report.txt

# Exe templates
exe_template 	= $(exe) --input $(samples)/$(1) --output $(results)/$(1) $(2)
report 			= $(exe) --input $(samples)/$(1) --output $(results)/$(1) --only-report $(2) | tee $(results)/$(1)/$(report_file)
clean_results	= find $(results)/$(1)/* -type d | xargs rm -rf


build: $(shell find lib -type f) main.cpp
	make -C build -j

# Sample 1 ========================================

sample1-report: build
	LOG_LEVEL=info $(call report,1)

sample1: build
	$(call clean_results,1)
	LOG_LEVEL=info $(call exe_template,1,--dump-edges)


# Sample 2 ========================================

sample2-report: build
	LOG_LEVEL=info $(call report,2)

sample2: build
	$(call clean_results,2)
	LOG_LEVEL=info $(call exe_template,2,--dump-edges)


# Sample 3 ========================================

sample3-report: build
	LOG_LEVEL=info $(call report,3,--stable-diff 40)

sample3: build
	$(call clean_results,3)
	LOG_LEVEL=info $(call exe_template,3,--stable-diff 40 --dump-edges)


# Sample 4 ========================================

sample4_params = --baseline-edges-union intersect --state-diff 20 --stable-diff 20 --curvature 20

sample4-report: build
	LOG_LEVEL=info $(call report,4,$(sample4_params))

sample4: build
	$(call clean_results,4)
	LOG_LEVEL=info $(call exe_template,4,$(sample4_params))

# Sample 5 ========================================

sample5_params = --state-diff 40 --stable-diff 40 --curvature 40

sample5-report: build
	LOG_LEVEL=info $(call report,5,$(sample5_params))

sample5: build
	$(call clean_results,4)
	LOG_LEVEL=info $(call exe_template,5,$(sample5_params))

# Dev ==============================================

dev: build
	LOG_LEVEL=info $(call exe_template,4,--filter baseline --edges-union intersect)

reports: sample1-report sample2-report sample3-report sample4-report sample5-report
samples: sample1 sample2 sample3 sample4 sample5


