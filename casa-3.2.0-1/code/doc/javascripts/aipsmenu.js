//# aipsmenu.js:  javascript for producing html menus
//# Copyright (C) 1999
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

//
// Useage: (Note may need relative path to scripts directory
//          First argument is relative path to docs tree
//          Second argument is color to make menu
//
// <SCRIPT LANGUAGE = "JavaScript1.2" src="javascripts/menu.js"></script>
// <SCRIPT LANGUAGE = "JavaScript1.2" src="javascripts/aipsmenu.js"></script>
// <SCRIPT LANGUAGE = "JavaScript1.2">
// <!--
//  // Function need to place directory relative to docs tree.
//  aMenu = new aipsMenu("../..", "#00009C");
//  function onLoad() {
//     if (document.layers) {
//         aMenu.loadMenus();
//     }
//  }
// //-->
// </SCRIPT>
//



function aipsMenu(toDoc, myColor) {
   this.toDoc = toDoc;    // Relative Path to docs tree
   this.myColor = myColor;// Color to make the menu
   this.showAipsMenu = showAipsMenu;
   this.loadMenus = loadMenus;


/*
     // topWin first part of string to specify location


   if(navigator.appName == "Netscape"){
     window.progMenu = new Menu("progMenu")
     window.userMenu = new Menu("userMenu");
     window.gsMenu = new Menu("gsMenu");
     window.glishMenu = new Menu("glishMenu");
     window.learnMenu = new Menu("learnMenu");
     window.commMenu = new Menu("commMenu");
     topWin = new String("top.window.location='"+this.toDoc);

     // where2 is just a dummy string to make things easier to read

     // Programmer Documentation menu
         where2 = topWin+"/reference/HowTos/HowTos.html'";
     progMenu.addMenuItem("General Help", where2);
         where2 = topWin+"/reference/Coding/Coding.html'";
     progMenu.addMenuItem("Programmers\'s Manual", where2);
         where2 = topWin+"/reference/System/System.html'";
     progMenu.addMenuItem("System Manual", where2);
         where2 = topWin+"/aips/aips.html'";
     progMenu.addMenuItem("Package aips", where2);
         where2 = topWin+"/trial/implement'";
     progMenu.addMenuItem("Trial", where2);
         where2 = topWin+"/html/qag.html'";
     progMenu.addMenuItem("Quality Assurance Group", where2);
         where2 = topWin+"/programmer/source.html'";
     progMenu.addMenuItem("Source Code", where2);
         where2 = topWin+"/programmer/srcfiletemplates.html'";
     progMenu.addMenuItem("Source-File Templates", where2);
         where2 = topWin+"/programmer/CCnotes.html'";
     progMenu.addMenuItem("Developer Notes", where2);
         
        // Specify some menu properties
     progMenu.fontWeight = "bold";             // Nice big letters
     progMenu.fontColor = "white";             // Make-em white to start
     progMenu.fontColorHilite ="yellow";       // Turn'em yellow when selected
     progMenu.menuItemBgColor = this.myColor;  // Set the background colour


     // User Documentation menu
	where2 = topWin+"/user/Refman/Refman.html'";
     userMenu.addMenuItem("User Reference Manual", where2);
          where2 = topWin+"/recipes/index.html'";
     userMenu.addMenuItem("Recipes", where2);
          where2 = topWin+"/glossary/glossary.html'";
     userMenu.addMenuItem("Glossary", where2);
          where2 = topWin+"/glish/glish.html'";
     userMenu.addMenuItem("Glish", where2);
     userMenu.fontWeight = "bold";
     userMenu.fontColor = "white";
     userMenu.menuItemBgColor = this.myColor;
     userMenu.fontColorHilite ="yellow";

     //Getting Started menu
          where2 = topWin+new String("/user/gettingstarted/gettingstarted.html'");
     gsMenu.addMenuItem("in AIPS++", where2);
          where2 = topWin+"/notes/195/195.html'";
     gsMenu.addMenuItem("with Glish", where2);
          where2 = topWin+"/reference/install.html'";
     gsMenu.addMenuItem("Obtaining", where2);
     gsMenu.fontWeight = "bold";
     gsMenu.fontColor = "white";
     gsMenu.menuItemBgColor = this.myColor;
     gsMenu.fontColorHilite ="yellow";

     //Glish menu
           where2 = topWin+"/notes/195/195.html'";
     glishMenu.addMenuItem("Getting Started", where2);
          where2 = topWin+"/reference/Glish/Glish.html'";
     glishMenu.addMenuItem("Glish User\'s Manual", where2);
         where2 = "top.window.location='http://www.cv.nrao.edu/glish/announce.html'";
     glishMenu.addMenuItem("Announcements", where2);
         where2 = "top.window.location='http://www.cv.nrao.edu/glish/papers/index.html'";
     glishMenu.addMenuItem("Articles", where2);
         where2 = "top.window.location='ftp://ftp.cv.nrao.edu/pub/glish'";
     glishMenu.addMenuItem("Source Code", where2);
     glishMenu.fontWeight = "bold";
     glishMenu.fontColor = "white";
     glishMenu.menuItemBgColor = this.myColor;
     glishMenu.fontColorHilite ="yellow";

     //Learn More menu
	  where2 = topWin+"/newsletters/index.html'";
     learnMenu.addMenuItem("Newsletters", where2);
	  where2 = topWin+"/notes/notes/notes.html'";
     learnMenu.addMenuItem("Notes", where2);
	 where2 = topWin+"/memos/memos/memos.html'";
     learnMenu.addMenuItem("Memos", where2);
	 where2 = topWin+"/papers/papers.html'";
     learnMenu.addMenuItem("Papers", where2);
               where2 = topWin+"/project/change-proposals/change-proposals.html'";
     learnMenu.addMenuItem("Change Requests", where2);
               where2 = topWin+"/project/quarterlyreports.html'";
     learnMenu.addMenuItem("Quarterly Reports", where2);
     learnMenu.fontWeight = "bold";
     learnMenu.fontColor = "white";
     learnMenu.menuItemBgColor = this.myColor;
     learnMenu.fontColorHilite ="yellow";

     //Contact us menu
	 where2 = topWin+"/contactus/trackabug.html'";
     commMenu.addMenuItem("Track a Bug", where2);
	 where2 = topWin+"/contactus/reportabug.html'";
     commMenu.addMenuItem("Report a Bug", where2);
	where2 = topWin+"/contactus/email.html'";
     commMenu.addMenuItem("E-mail reflectors", where2);
	 where2 = topWin+"/contactus/personnel.html'";
     commMenu.addMenuItem("Personnel", where2);
	where2 = topWin+"/contactus/jobs.html'";
     commMenu.addMenuItem("Job Opportunities", where2);
         where2 = topWin+"/contactus/mirror.html'";
     commMenu.addMenuItem("Mirror Sites", where2);
        
        
        
     commMenu.fontWeight = "bold";
     commMenu.fontColor = "white";
     commMenu.menuItemBgColor = this.myColor;
     commMenu.fontColorHilite ="yellow";

  // Flush the menus out
  commMenu.writeMenus();
  }
*/
}

function loadMenus() {             // Make the menus
      // no oop
}
function showAipsMenu(theMenu){
/*
   if(navigator.appName == "Netscape"){
      window.showMenu(theMenu, 0, 35);
   }
*/
}
