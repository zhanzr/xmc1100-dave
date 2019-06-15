<?xml version="1.0" encoding="ASCII"?>
<ResourceModel:App xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ResourceModel="http://www.infineon.com/Davex/Resource.ecore" name="ANALOG_IO" URI="http://resources/4.0.10/app/ANALOG_IO/0" description="Configures a port pin to use as analog input/output and digital input." version="4.0.10" minDaveVersion="4.0.0" instanceLabel="ANALOG_IO_0" appLabel="">
  <upwardMapList xsi:type="ResourceModel:RequiredApp" href="../../ADC_MEASUREMENT/v4_1_22/ADC_MEASUREMENT_0.app#//@requiredApps.2"/>
  <properties provideInit="true" sharable="true"/>
  <virtualSignals name="pin" URI="http://resources/4.0.10/app/ANALOG_IO/0/vs_analog_io_pad_pin" hwSignal="pad" hwResource="//@hwResources.0" visible="true">
    <upwardMapList xsi:type="ResourceModel:Connections" href="../../ADC_MEASUREMENT/v4_1_22/ADC_MEASUREMENT_0.app#//@connections.80"/>
  </virtualSignals>
  <virtualSignals name="hw_pull_ctrl" URI="http://resources/4.0.10/app/ANALOG_IO/0/vs_analog_io_hwp" hwSignal="hw_p" hwResource="//@hwResources.0" required="false"/>
  <virtualSignals name="pin_signal" URI="http://resources/4.0.10/app/ANALOG_IO/0/__pin_vs_analog_io_pad_pin" hwSignal="pin" hwResource="//@hwResources.1"/>
  <hwResources name="pin" URI="http://resources/4.0.10/app/ANALOG_IO/0/hwres_port_pin" resourceGroupUri="port/p/sv0/pad/*" solverVariable="true" mResGrpUri="port/p/sv0/pad/*">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/Port2/Port2_2.dd#//@provided.8"/>
    <solverVarMap index="2">
      <value variableName="sv0" solverValue="2"/>
    </solverVarMap>
    <solverVarMap index="2">
      <value variableName="sv0" solverValue="2"/>
    </solverVarMap>
  </hwResources>
  <hwResources name="pin" URI="http://resources/4.0.10/app/ANALOG_IO/0/__pin_hwres_port_pin" resourceGroupUri="devicepackage/0/35" constraintType="GLOBAL_RESOURCE" mResGrpUri="devicepackage/0/*">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/DEVICEPACKAGE/DEVICEPACKAGE_0.dd#//@provided.9"/>
  </hwResources>
  <connections URI="http://resources/4.0.10/app/ANALOG_IO/0/http://resources/4.0.10/app/ANALOG_IO/0/vs_analog_io_pad_pin/http://resources/4.0.10/app/ANALOG_IO/0/__pin_vs_analog_io_pad_pin" systemDefined="true" sourceSignal="pin" targetSignal="pin_signal" srcVirtualSignal="//@virtualSignals.0" targetVirtualSignal="//@virtualSignals.2"/>
  <connections URI="http://resources/4.0.10/app/ANALOG_IO/0/http://resources/4.0.10/app/ANALOG_IO/0/__pin_vs_analog_io_pad_pin/http://resources/4.0.10/app/ANALOG_IO/0/vs_analog_io_pad_pin" systemDefined="true" sourceSignal="pin_signal" targetSignal="pin" srcVirtualSignal="//@virtualSignals.2" targetVirtualSignal="//@virtualSignals.0"/>
</ResourceModel:App>
