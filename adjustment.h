#pragma once

class DeviceProxy;
class WorkingParameters;

// class Adjustment
class Adjustment {
public:
  Adjustment();
  virtual ~Adjustment();

  virtual bool Execute(DeviceProxy *device, WorkingParameters *parameters) = 0;
};