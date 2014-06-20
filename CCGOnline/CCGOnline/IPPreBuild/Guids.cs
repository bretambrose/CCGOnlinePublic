// Guids.cs
// MUST match guids.h
using System;

namespace InterminableProcesses.IPPreBuild
{
    static class GuidList
    {
        public const string guidIPPreBuildPkgString = "04c507ad-fd52-47bb-a747-5ad88964267b";
        public const string guidIPPreBuildCmdSetString = "19c9fe03-6f42-40d4-b7b0-447e3a16fb8e";

        public static readonly Guid guidIPPreBuildCmdSet = new Guid(guidIPPreBuildCmdSetString);
    };
}