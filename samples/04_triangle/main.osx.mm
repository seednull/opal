#include <Cocoa/Cocoa.h>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/CADisplayLink.h>
#include <cassert>

#include "app.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *) sender
{
	return YES;
}
@end

@interface WindowDelegate : NSObject <NSWindowDelegate>
@property Application *app;
@property NSSize size;
@end

@implementation WindowDelegate
- (NSSize) windowWillResize:(NSWindow *) sender toSize:(NSSize) size
{
	NSRect frame = sender.frame;
	frame.size = size;

	NSRect content = [sender contentRectForFrameRect: frame];
	self.size = content.size;

	return size;
}

- (void) windowDidResize:(NSNotification *) notification
{
	assert(self.app);
	self.app->resize(self.size.width, self.size.height);
}

- (void) windowWillClose:(NSWindow *) sender
{
	assert(self.app);
	self.app->shutdown();
}
@end

@interface WindowView : NSView
@property Application *app;
@end

@implementation WindowView
- (instancetype)initWithFrame:(NSRect) rect
{
	self = [super initWithFrame: rect];

	self.wantsLayer = YES;
	self.layer = [CAMetalLayer layer];

	CADisplayLink *link = [self displayLinkWithTarget:self selector:@selector(update:)];
	[link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

	return self;
}

- (void) update:(CADisplayLink *) sender
{
	assert(self.app);

	float dt = static_cast<float>(sender.targetTimestamp - sender.timestamp);

	self.app->update(dt);
	self.app->render();
	self.app->present();
}
@end

int main(int argc, const char *argv[])
{
	NSRect screen = [[NSScreen mainScreen] frame];

	int w = 800;
	int h = 600;
	int x = screen.size.width / 2 - w / 2;
	int y = screen.size.height / 2 - h / 2;

	NSRect view = NSMakeRect(x, y, w, h);

	Application app;

	[NSApplication sharedApplication];
	NSApp.delegate = [[AppDelegate alloc] init];
	NSApp.activationPolicy = NSApplicationActivationPolicyRegular;

	[NSApp finishLaunching];

	NSWindow *window = [
		[NSWindow alloc]
		initWithContentRect: view
		styleMask: (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable)
		backing: NSBackingStoreBuffered
		defer: NO
	];

	WindowDelegate *window_delegate = [[WindowDelegate alloc] init];
	window_delegate.app = &app;

	WindowView *window_view = [[WindowView alloc] initWithFrame: view];
	window_view.app = &app;

	window.delegate = window_delegate;
	window.contentView = window_view;
	window.acceptsMouseMovedEvents = YES;
	window.title = @"Opal Sample (04_triangle)";

	[window makeKeyAndOrderFront:nil];

	app.init(window_view.layer, w, h);
	[NSApp run];

	return 0;
}
