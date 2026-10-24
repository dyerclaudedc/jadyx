# SPDX-FileCopyrightText: Copyright 2026 Jadyx Project
# SPDX-License-Identifier: GPL-3.0-or-later
import zipfile, pathlib, warnings
root=pathlib.Path('build-uwp/test-results/updater/packages');root.mkdir(parents=True,exist_ok=True)
valid='<Package xmlns="http://schemas.microsoft.com/appx/manifest/foundation/windows10"><Identity Name="DyerPlay" Publisher="CN=DyerPlay" Version="0.3.6.0" ProcessorArchitecture="x64" /></Package>'
fixtures={
 'valid':valid,'valid-bom':'﻿<?xml version="1.0" encoding="utf-8"?>\r\n'+valid,
 'wrong-version':valid.replace('0.3.6.0','0.3.7.0'),
 'wrong-name':valid.replace('Name="DyerPlay"','Name="Other"'),
 'wrong-publisher':valid.replace('DyerPlay','Other'),
 'wrong-architecture':valid.replace('x64','arm64'),
 'wrong-namespace':valid.replace('foundation/windows10','foundation/other'),
 'comment-only':valid.replace('<Identity','<!-- <Identity').replace('/>','/> -->'),
 'duplicate-attribute':valid.replace('Name="DyerPlay"','Name="DyerPlay" Name="Other"'),
 'doctype':'<!DOCTYPE Package [<!ENTITY test SYSTEM "file:///C:/must-not-open">]>'+valid,
 'empty-signature':valid,'missing-signature':valid,'duplicate-manifest':valid,
 'missing-manifest':None,'truncated':valid,
}
with warnings.catch_warnings():
 warnings.simplefilter('ignore',UserWarning)
 for name,xml in fixtures.items():
  path=root/(name+'.appx')
  with zipfile.ZipFile(path,'w',compression=zipfile.ZIP_DEFLATED) as z:
   if xml is not None:z.writestr('AppxManifest.xml',xml)
   if name=='duplicate-manifest':z.writestr('AppxManifest.xml',xml)
   if name!='missing-signature':z.writestr('AppxSignature.p7x',b'' if name=='empty-signature' else b'PKCXfixture')
  if name=='truncated':path.write_bytes(path.read_bytes()[:len(path.read_bytes())//2])
print('APPX structural fixtures:',len(fixtures))
