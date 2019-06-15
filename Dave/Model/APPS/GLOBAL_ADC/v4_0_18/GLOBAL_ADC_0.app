<?xml version="1.0" encoding="ASCII"?>
<ResourceModel:App xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ResourceModel="http://www.infineon.com/Davex/Resource.ecore" name="GLOBAL_ADC" URI="http://resources/4.0.18/app/GLOBAL_ADC/0" description="Initializes VADC GLOBAL and GROUP resources." version="4.0.18" minDaveVersion="4.0.0" instanceLabel="GLOBAL_ADC_0" appLabel="">
  <upwardMapList xsi:type="ResourceModel:RequiredApp" href="../../ADC_MEASUREMENT/v4_1_22/ADC_MEASUREMENT_0.app#//@requiredApps.0"/>
  <properties singleton="true" provideInit="true" sharable="true"/>
  <virtualSignals name="global_signal" URI="http://resources/4.0.18/app/GLOBAL_ADC/0/vs_global_adc_global" hwSignal="global_signal" hwResource="//@hwResources.0" visible="true">
    <upwardMapList xsi:type="ResourceModel:Connections" href="../../ADC_MEASUREMENT/v4_1_22/ADC_MEASUREMENT_0.app#//@connections.53"/>
  </virtualSignals>
  <requiredApps URI="http://resources/4.0.18/app/GLOBAL_ADC/0/appres_clock" requiredAppName="CLOCK_XMC1" requiringMode="SHARABLE">
    <downwardMapList xsi:type="ResourceModel:App" href="../../CLOCK_XMC1/v4_0_20/CLOCK_XMC1_0.app#/"/>
  </requiredApps>
  <hwResources name="Global" URI="http://resources/4.0.18/app/GLOBAL_ADC/0/hwres_vadc_global" resourceGroupUri="peripheral/vadc/*/global" mResGrpUri="peripheral/vadc/*/global">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/VADC/VADC_0.dd#//@provided.2"/>
  </hwResources>
  <hwResources name="Group0" URI="http://resources/4.0.18/app/GLOBAL_ADC/0/hwres_vadc_group0" resourceGroupUri="peripheral/vadc/*/group/0/config" mResGrpUri="peripheral/vadc/*/group/0/config">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/VADC/VADC_0.dd#//@provided.14"/>
  </hwResources>
  <hwResources name="Group1" URI="http://resources/4.0.18/app/GLOBAL_ADC/0/hwres_vadc_group1" resourceGroupUri="peripheral/vadc/*/group/1/config" mResGrpUri="peripheral/vadc/*/group/1/config">
    <downwardMapList xsi:type="ResourceModel:ResourceGroup" href="../../../HW_RESOURCES/VADC/VADC_0.dd#//@provided.13"/>
  </hwResources>
</ResourceModel:App>
