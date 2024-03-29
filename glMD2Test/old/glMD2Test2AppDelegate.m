//
//  glMD2Test2AppDelegate.m
//  glMD2Test2
//
//  Created by Adam Dann on 25/07/08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//

#import "glMD2Test2AppDelegate.h"
#import "EAGLView.h"

@implementation glMD2Test2AppDelegate

@synthesize window;
@synthesize glView;

- (void)applicationDidFinishLaunching:(UIApplication *)application {

	glView.animationInterval = 1.0 / 60.0;
	[glView startAnimation];
}


- (void)applicationWillResignActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 5.0;
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
	glView.animationInterval = 1.0 / 60.0;
}

- (void)dealloc {
	[window release];
	[glView release];
	[super dealloc];
}

@end
