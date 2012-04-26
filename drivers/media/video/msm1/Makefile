GCC_VERSION      := $(shell $(CONFIG_SHELL) $(PWD)/scripts/gcc-version.sh $(CROSS_COMPILE)gcc)
ifeq ($(GCC_VERSION),0404)
CFLAGS_REMOVE_msm_vfe8x.o = -Wframe-larger-than=1024
endif

ifeq ($(CONFIG_MSM_CAMERA_V4L2),y)
  EXTRA_CFLAGS += -Idrivers/media/video/msm/csi
  EXTRA_CFLAGS += -Idrivers/media/video/msm/io
  EXTRA_CFLAGS += -Idrivers/media/video/msm/sensors
  obj-$(CONFIG_MSM_CAMERA) += msm_isp.o msm.o msm_mem.o msm_mctl.o msm_mctl_buf.o msm_mctl_pp.o
  obj-$(CONFIG_MSM_CAMERA) += io/ sensors/ actuators/ csi/
else
  obj-$(CONFIG_MSM_CAMERA) += msm_camera.o
endif
obj-$(CONFIG_MSM_CAMERA) += msm_axi_qos.o gemini_8x60/
obj-$(CONFIG_MSM_CAMERA_FLASH) += flash.o
obj-$(CONFIG_ARCH_MSM_ARM11) += msm_vfe7x.o msm_io7x.o
ifeq ($(CONFIG_MSM_CAMERA_V4L2),y)
  obj-$(CONFIG_ARCH_MSM7X27A) += msm_vfe7x27a_v4l2.o msm_io_7x27a_v4l2.o
else
  obj-$(CONFIG_ARCH_MSM7X27A) += msm_vfe7x27a.o msm_io_7x27a.o
endif
obj-$(CONFIG_ARCH_QSD8X50) += msm_vfe8x.o msm_vfe8x_proc.o msm_io8x.o
ifeq ($(CONFIG_MSM_CAMERA_V4L2),y)
  obj-$(CONFIG_ARCH_MSM8X60) += msm_io_vfe31_v4l2.o msm_vfe31_v4l2.o msm_vpe.o
  obj-$(CONFIG_ARCH_MSM7X30) += msm_io_vfe31_v4l2.o msm_vfe31_v4l2.o msm_vpe.o msm_axi_qos.o
else
  obj-$(CONFIG_ARCH_MSM8X60) += msm_vfe31.o msm_io_8x60.o msm_vpe1.o
  obj-$(CONFIG_ARCH_MSM7X30) += msm_vfe31.o msm_io_vfe31.o msm_vpe1.o
endif
obj-$(CONFIG_ARCH_MSM8960) += msm_io_8960.o msm_vfe32.o msm_vpe.o
ifneq ($(CONFIG_MSM_CAMERA_V4L2),y)
  obj-$(CONFIG_S5K4E1) += s5k4e1.o s5k4e1_reg.o
endif
obj-$(CONFIG_FB_MSM_WRITEBACK_MSM_PANEL) += wfd/

obj-$(CONFIG_MACH_SHOOTER) += s5k6aafx.o s5k6aafx_reg_shooter.o s5k6aafx_reg_shooter_cob.o
obj-$(CONFIG_MACH_SHOOTER_U) += s5k6aafx.o s5k6aafx_reg_shooter.o s5k6aafx_reg_shooter_cob.o
obj-$(CONFIG_MACH_SHOOTER_U) += qs_s5k4e1.o qs_s5k4e1_reg.o
obj-$(CONFIG_SP3D) += sp3d.o sp3d_reg_3d.o sp3d_reg_2d.o sp3d_reg_com.o sp3d_reg_cal.o
