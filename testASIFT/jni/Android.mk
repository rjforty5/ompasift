# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# the purpose of this sample is to demonstrate how one can
# generate two distinct shared libraries and have them both
# uploaded in
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := ASIFT
LOCAL_SRC_FILES := ASIFT_Dll.cpp compute_asift_keypoints.cpp compute_asift_matches.cpp \
demo_lib_sift.cpp domain.cpp filter.cpp flimage.cpp fproj.cpp frot.cpp library.cpp numerics1.cpp\
orsa.cpp splines.cpp\

include $(BUILD_SHARED_LIBRARY)
