// Copyright (C) 2021-2024  CEA, EDF
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

#pragma once

#include "MED_Wrapper.hxx"
#include "MED_WrapperDef.hxx"

namespace MED
{
  class TFileInternal
  {
  public:
    virtual ~TFileInternal() = default;
    virtual void Open(EModeAcces theMode, TErr* theErr = nullptr) = 0;
    virtual void Close() = 0;
    virtual const TIdt& Id() const = 0;
  };

  class MEDWRAPPER_EXPORT MEDIDTHoder : public TFileInternal
  {
  protected:
    MEDIDTHoder(bool *isClosedStatus = nullptr):_isClosedStatus(isClosedStatus) { }
    void UnRefFid()
    {
      if (--myCount == 0)
      {
        MEDfileClose(myFid);
        myIsClosed = true;
        if(_isClosedStatus)
          *_isClosedStatus = true;
      }
    }
  public:
    const TIdt& Id() const override;
    ~MEDIDTHoder() { this->UnRefFid(); }
    void Close() override { this->UnRefFid(); }
  protected:
    TInt myCount = 0;
    TIdt myFid = 0;
    bool myIsClosed = false;
    bool *_isClosedStatus = nullptr;
  };

  class MEDWRAPPER_EXPORT TFileDecorator : public TFileInternal
  {
  public:
    TFileDecorator(TFileInternal *effective):_effective(effective) { }
    void Open(EModeAcces theMode, TErr* theErr = nullptr) override { if(_effective) _effective->Open(theMode,theErr); }
    void Close() override { if(_effective) _effective->Close(); }
    const TIdt& Id() const override { if(_effective) return _effective->Id(); EXCEPTION(std::runtime_error, "TFileDecorator - GetFid() : no effective TFile !"); }
    ~TFileDecorator() { delete _effective; }
  private:
    TFileInternal *_effective = nullptr;
  };

  class MEDWRAPPER_EXPORT TMemFile : public MEDIDTHoder
  {
  public:
    TMemFile(void **data, std::size_t *sz, bool* isClosedStatus):MEDIDTHoder(isClosedStatus),_data(data),_sz(sz) { memfile.app_image_ptr=*data; memfile.app_image_size=*sz; }
    ~TMemFile() { UnRefFid(); if(myIsClosed) { *_data = memfile.app_image_ptr; *_sz = memfile.app_image_size; } }
    void Open(EModeAcces theMode, TErr* theErr = nullptr) override;
    void *getData() const { return memfile.app_image_ptr; }
    std::size_t getSize() const { return memfile.app_image_size; }
  private:
    void **_data = nullptr;
    std::size_t * _sz = nullptr;
    med_memfile memfile = MED_MEMFILE_INIT;
  };

  class MEDWRAPPER_EXPORT TFile : public MEDIDTHoder
  {
  public:
    TFile(const std::string& theFileName, TInt theMajor=-1, TInt theMinor=-1);
    void Open(EModeAcces theMode, TErr* theErr = nullptr) override;
  protected:
    std::string myFileName;
    TInt myMajor;
    TInt myMinor;
  };
}
