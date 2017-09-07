ZBAR_TEST_LICENSE = Barco
ZBAR_TEST_VERSION = HEAD
ZBAR_TEST_SITE = /home/lodco/analyser/zbar-test
ZBAR_TEST_SITE_METHOD = local
ZBAR_TEST_INSTALL_STAGING = YES
ZBAR_TEST_INSTALL_TARGET = NO
ZBAR_TEST_DEPENDENCIES += qt5base libunified-logger

define ZBAR_TEST_BUILD_CMDS
  ( pushd $(@D) > /dev/null && \
  $(HOST_DIR)/usr/bin/qmake "BUILDROOT=yes" && \
  $(MAKE)  && \
  popd > /dev/null )
endef

$(eval $(generic-package))
