// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace UnrealBuildTool
{
	public class Unity
	{
		/// <summary>
		/// Given a set of C++ files, generates another set of C++ files that #include all the original
		/// files, the goal being to compile the same code in fewer translation units.
		/// The "unity" files are written to the CompileEnvironment's OutputDirectory.
		/// </summary>
		/// <param name="CPPFiles">The C++ files to #include.</param>
		/// <param name="CompileEnvironment">The environment that is used to compile the C++ files.</param>
		/// <param name="BaseName">Base name to use for the Unity files</param>
		/// <returns>The "unity" C++ files.</returns>
		public static List<FileItem> GenerateUnityCPPs(
			List<FileItem> CPPFiles, 
			CPPEnvironment CompileEnvironment,
			string BaseName
			)
		{
			UEBuildPlatform BuildPlatform = UEBuildPlatform.GetBuildPlatformForCPPTargetPlatform(CompileEnvironment.Config.TargetPlatform);

			// Figure out size of all input files combined. We use this to determine whether to use larger unity threshold or not.
			long TotalBytesInCPPFiles = 0;
			foreach( FileItem CPPFile in CPPFiles )
			{
				TotalBytesInCPPFiles += CPPFile.Info.Length;
			}

			// We have an increased threshold for unity file size if, and only if, all files fit into the same unity file. This
			// is beneficial when dealing with PCH files. The default PCH creation limit is X unity files so if we generate < X 
			// this could be fairly slow and we'd rather bump the limit a bit to group them all into the same unity file.
			bool bForceIntoSingleUnityFile = BuildConfiguration.bStressTestUnity;			
			if( (TotalBytesInCPPFiles < BuildConfiguration.NumIncludedBytesPerUnityCPP * 2)
			// Optimization only makes sense if PCH files are enabled.
			&&	CompileEnvironment.ShouldUsePCHs() )
			{
				bForceIntoSingleUnityFile = true;
			}

			// Figure out how many unity files there are going to be total.
			int NumUnityFiles = 0;
			int InputFileIndex = 0;
			while (InputFileIndex < CPPFiles.Count)
			{
				long NumIncludedBytesInThisOutputFile = 0;
				//@warning: this condition is mirrored below
				while(	InputFileIndex < CPPFiles.Count &&
						(bForceIntoSingleUnityFile ||
						NumIncludedBytesInThisOutputFile < BuildConfiguration.NumIncludedBytesPerUnityCPP))
				{
                    bool ForceAlone = CPPFiles[InputFileIndex].AbsolutePath.Contains(".GeneratedWrapper.");
                    if (ForceAlone && !bForceIntoSingleUnityFile && NumIncludedBytesInThisOutputFile > 0)
                    {
                        break;
                    }
                    NumIncludedBytesInThisOutputFile += CPPFiles[InputFileIndex].Info.Length;
                    InputFileIndex++;
                    if (ForceAlone && !bForceIntoSingleUnityFile)
                    {
                        break;
                    }
                }
				NumUnityFiles++;
			}

			// Create a set of CPP files that combine smaller CPP files into larger compilation units, along with the corresponding 
			// actions to compile them.
			InputFileIndex = 0;
			int CurrentUnityFileCount = 1;
			List<FileItem> UnityCPPFiles = new List<FileItem>();
			var ToolChain = UEToolChain.GetPlatformToolChain(CompileEnvironment.Config.TargetPlatform);

			string PCHHeaderNameInCode = CPPFiles[0].PCHHeaderNameInCode;
			if( CompileEnvironment.Config.PrecompiledHeaderIncludeFilename != null )
			{
				PCHHeaderNameInCode = ToolChain.ConvertPath( CompileEnvironment.Config.PrecompiledHeaderIncludeFilename );

				// Generated unity .cpp files always include the PCH using an absolute path, so we need to update
				// our compile environment's PCH header name to use this instead of the text it pulled from the original
				// C++ source files
				CompileEnvironment.Config.PCHHeaderNameInCode = PCHHeaderNameInCode;
			}

			while (InputFileIndex < CPPFiles.Count)
			{
				StringWriter OutputUnityCPPWriter = new StringWriter();
				StringWriter OutputUnityCPPWriterExtra = null;
				// add an extra file for UBT to get the #include dependencies from
				if (BuildPlatform.RequiresExtraUnityCPPWriter() == true)
				{
					OutputUnityCPPWriterExtra = new StringWriter();
				}
				
				OutputUnityCPPWriter.WriteLine("// This file is automatically generated at compile-time to include some subset of the user-created cpp files.");

				// Explicitly include the precompiled header first, since Visual C++ expects the first top-level #include to be the header file
				// that was used to create the PCH.
				if (CompileEnvironment.Config.PrecompiledHeaderIncludeFilename != null)
				{
					OutputUnityCPPWriter.WriteLine("#include \"{0}\"", PCHHeaderNameInCode);
					if (OutputUnityCPPWriterExtra != null)
					{
						OutputUnityCPPWriterExtra.WriteLine("#include \"{0}\"", PCHHeaderNameInCode);
					}
				}

				// Add source files to the unity file until the number of included bytes crosses a threshold.
				long NumIncludedBytesInThisOutputFile = 0;
				var NumInputFilesInThisOutputFile = 0;
                string FileDescription = "";
				//@warning: this condition is mirrored above
				while(	InputFileIndex < CPPFiles.Count &&
						(bForceIntoSingleUnityFile ||
						NumIncludedBytesInThisOutputFile < BuildConfiguration.NumIncludedBytesPerUnityCPP))
				{
                    bool ForceAlone = CPPFiles[InputFileIndex].AbsolutePath.Contains(".GeneratedWrapper.");
                    if (ForceAlone && !bForceIntoSingleUnityFile && NumIncludedBytesInThisOutputFile > 0)
                    {
                        break;
                    }
                    FileItem CPPFile = CPPFiles[InputFileIndex];
					OutputUnityCPPWriter.WriteLine("#include \"{0}\"", ToolChain.ConvertPath(CPPFile.AbsolutePath));
                    if (OutputUnityCPPWriterExtra != null)
                    {
                        OutputUnityCPPWriterExtra.WriteLine("#include \"{0}\"", CPPFile.AbsolutePath);
                    }

					NumIncludedBytesInThisOutputFile += CPPFile.Info.Length;
                    InputFileIndex++;
					NumInputFilesInThisOutputFile++;
                    FileDescription += Path.GetFileName(CPPFile.AbsolutePath) + " + ";
                    if (ForceAlone && !bForceIntoSingleUnityFile)
                    {
                        break;
                    }
                }
                // Remove trailing " + "
                FileDescription = FileDescription.Remove( FileDescription.Length - 3);

				// Write the unity file to the intermediate folder.
				string ProgressString = "." + CurrentUnityFileCount.ToString() + "_of_" + NumUnityFiles.ToString();
				string UnityCPPFilePath = Path.Combine(
					CompileEnvironment.Config.OutputDirectory,
					string.Format("Module.{0}{1}.cpp",
						BaseName,
						NumUnityFiles > 1 ? ProgressString : ""
						)
					);
				CurrentUnityFileCount++;

				FileItem UnityCPPFile = FileItem.CreateIntermediateTextFile(UnityCPPFilePath, OutputUnityCPPWriter.ToString());
				UnityCPPFile.RelativeCost = NumIncludedBytesInThisOutputFile;
				UnityCPPFile.Description = FileDescription;
				UnityCPPFile.PCHHeaderNameInCode = PCHHeaderNameInCode;
                UnityCPPFiles.Add(UnityCPPFile);

				// write out the extra file
				if (OutputUnityCPPWriterExtra != null)
				{
					FileItem.CreateIntermediateTextFile(UnityCPPFilePath + ".ex", OutputUnityCPPWriterExtra.ToString());
				}
			}

			return UnityCPPFiles;
		}
	}
}
