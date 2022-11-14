#ifndef CAPTURE_H
#define CAPTURE_H

HRESULT CreateVideoDeviceSource(IMFMediaSource **OutSource)
{
  *OutSource = NULL;
  
  IMFMediaSource *Source = NULL;
  IMFAttributes *Attributes = NULL;
  IMFActivate **Devices = NULL;
  
  // Create an attribute store to specify the enumeration parameters.
  HRESULT Status = MFCreateAttributes(&Attributes, 1);
  Assert(!FAILED(Status));
  
  // Source type: video capture devices
  Status = IMFAttributes_SetGUID(Attributes, &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
                                 &MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
  Assert(!FAILED(Status));
  
  // Enumerate devices.
  UINT32 Count;
  Status = MFEnumDeviceSources(Attributes, &Devices, &Count);
  Assert(!FAILED(Status));
  
  if (Count == 0) { Status = E_FAIL; goto done; }
  
  // Create the media source object.
  IMFActivate_ActivateObject(Devices[0], &IID_IMFMediaSource, &Source);
  Assert(!FAILED(Status));
  
  *OutSource = Source;
  IMFMediaSource_AddRef(*OutSource);
  
  done:
  if(Attributes) IMFAttributes_Release(Attributes);
  
  for (DWORD i = 0; i < Count; i++)
  {
    if(Devices[i]) IMFActivate_Release(Devices[i]);
  }
  CoTaskMemFree(Devices);
  if(Source) IMFMediaSource_Release(Source);
  return Status;
}

#endif //CAPTURE_H
