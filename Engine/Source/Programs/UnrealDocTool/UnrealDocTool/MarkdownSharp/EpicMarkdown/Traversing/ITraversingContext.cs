﻿// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

namespace MarkdownSharp.EpicMarkdown.Traversing
{
    public interface ITraversingContext
    {
        void PreChildrenVisit(ITraversable traversedElement);
        void PostChildrenVisit(ITraversable traversedElement);
    }
}