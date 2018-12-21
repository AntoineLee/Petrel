//
//  QHVCEditCoreImagePreview.m
//  QHVCEditKit
//
//  Created by liyue-g on 2017/12/11.
//  Copyright © 2017年 liyue-g. All rights reserved.
//

#import "QHVCEditCIPreview.h"
#import "QHVCEditLogger.h"
#import <GLKit/GLKit.h>
#include <notify.h>
#import "QHVCLifeCycle.h"
#import "QHVCEditUtilsSet.h"
#import "QHVCEditConfig.h"

@interface QHVCEditCIPreview ()<QHVCLifeCycleDelegate>

@property (nonatomic, assign) CGFloat bgColorRed;
@property (nonatomic, assign) CGFloat bgColorGreen;
@property (nonatomic, assign) CGFloat bgColorBlue;
@property (nonatomic, assign) CGFloat bgColorAlpha;
@property (atomic, assign) BOOL disable;

@property (atomic, strong) GLKView* preview;
@property (atomic, strong) CIContext* ciContext;
@property (atomic, strong) EAGLContext* eaglContext;
@property (atomic, assign) CGRect previewBounds;
@property (atomic, assign) BOOL stopDrawing;
@property (nonatomic, retain) CIImage* previewImage;
@property (nonatomic, strong) dispatch_queue_t previewQueue;

@property (nonatomic, strong) CADisplayLink* displayLink;
@property (atomic,    retain) NSMutableArray* waitToRenderArray;
@property (nonatomic, retain) NSCondition* condition;

@property (nonatomic, assign) NSInteger lastFrameTimestamp;

@end

@implementation QHVCEditCIPreview

- (void)dealloc
{
    self.disable = YES;
    if (_preview)
    {
        glFlush();
        [_preview deleteDrawable];
        [_preview removeFromSuperview];
        [QHVCLifeCycle unregisterLifeCycleListener:self];
    }
    
    if ([EAGLContext currentContext] == self.eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
    }
    
    if (self.displayLink)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
    }
}

- (void)free
{
    self.disable = YES;
    self.ciContext = nil;
    self.eaglContext = nil;
    if (_preview)
    {
        glFlush();
        [_preview deleteDrawable];
        [_preview removeFromSuperview];
        [QHVCLifeCycle unregisterLifeCycleListener:self];
    }
    
    if ([EAGLContext currentContext] == self.eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
    }
    
    if (self.displayLink)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
    }
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        [self initContext];
        [QHVCLifeCycle registerLifeCycleListener:self];
        self.previewQueue = dispatch_queue_create("QHVCEditPreviewQueue", NULL);
        dispatch_set_target_queue(self.previewQueue, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0));
        
        self.displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderFrame)];
        self.displayLink.paused = YES;
        [self.displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        self.waitToRenderArray = [[NSMutableArray alloc] initWithCapacity:0];
        self.condition = [[NSCondition alloc] init];
        
        [self setBackgroundColor:[UIColor clearColor]];
        self.layer.allowsEdgeAntialiasing = YES;
        
        self.inputCount = 0;
        self.renderedCount = 0;
    }
    
    return self;
}

- (void)screenLocked:(NSNotification *)notification
{
    _stopDrawing = YES;
}

- (void)willEnterForeground:(NSNotification *)notification
{
    _stopDrawing = NO;
}

- (void)willResignActive:(NSNotification *)notification
{
    _stopDrawing = YES;
}

- (void)didBecomeActive:(NSNotification *)notification
{
    _stopDrawing = NO;
}

- (void)initContext
{
    if (!self.eaglContext)
    {
        [EAGLContext setCurrentContext:nil];
        EAGLContext* sharedContext = [[QHVCEditConfig sharedInstance] sharedImageProcessingContext];
        self.eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:[sharedContext sharegroup]];
    }
    
    if (!self.ciContext && self.eaglContext)
    {
        [EAGLContext setCurrentContext:self.eaglContext];
        self.ciContext = [CIContext contextWithEAGLContext:self.eaglContext options:nil];
    }
}

- (void)setPreviewFrame:(CGRect)frame
{
    if (!CGRectEqualToRect(self.preview.frame, frame))
    {
        [self.preview deleteDrawable];
        [self.preview removeFromSuperview];
        
        if (!self.preview)
        {
            self.preview = [[GLKView alloc] initWithFrame:frame context:self.eaglContext];
            [self.preview setBackgroundColor:[UIColor clearColor]];
            [self.preview setUserInteractionEnabled:NO];
            self.preview.enableSetNeedsDisplay = NO;
            self.preview.layer.allowsEdgeAntialiasing = YES;
        }
        
        [self.preview setFrame:frame];
        [self.preview bindDrawable];
        [self addSubview:self.preview];
        [self sendSubviewToBack:self.preview];
        
        self.previewBounds = CGRectZero;
        _previewBounds.size.width = self.preview.drawableWidth;
        _previewBounds.size.height = self.preview.drawableHeight;
        _previewFrame = frame;
    }
}

- (CIImage *)getCurrentFrame
{
    return self.previewImage;
}

- (void)setBackgroundColorRed:(CGFloat)redComponent green:(CGFloat)greenComponent blue:(CGFloat)blueComponent alpha:(CGFloat)alphaComponent
{
    self.bgColorRed = redComponent;
    self.bgColorGreen = greenComponent;
    self.bgColorBlue = blueComponent;
    self.bgColorAlpha = alphaComponent;
}

- (void)setFrame:(CGRect)frame
{
    [super setFrame:frame];
    [self setPreviewFrame:frame];
}

- (void)setDisable
{
    self.disable = YES;
}

- (void)processImage:(CIImage *)image timestampMs:(NSInteger)timestampMs userData:(id)userData
{
    if (self.disable)
    {
        return;
    }
    
    if (_stopDrawing)
    {
        return;
    }
    
    if (!image)
    {
        return;
    }
    
    [self.condition lock];
    
    if (self.lastFrameTimestamp == 0)
    {
        self.lastFrameTimestamp = timestampMs;
    }
    
    //45fps输出
    if (/*abs(self.lastFrameTimestamp - timestampMs) < 22 &&*/ [self.waitToRenderArray count] > 3)
    {
        LogDebug(@"preview drop frame, waitToRenderCount = %d,", (int)[self.waitToRenderArray count]);
        [self.condition unlock];
        return;
    }
    
    self.inputCount ++;
    [self.waitToRenderArray addObject:image];
    self.lastFrameTimestamp = timestampMs;
    
    if ([self.displayLink isPaused])
    {
        [self.displayLink setPaused:NO];
    }
    [self.condition unlock];
}

- (void)renderFrame
{
    if (!self.eaglContext || !self.ciContext)
    {
        return;
    }
    
    [self.condition lock];
    
    CIImage* image = [self.waitToRenderArray firstObject];
    if (!image)
    {
        [self.condition unlock];
        return;
    }
    [self.waitToRenderArray removeObjectAtIndex:0];
    
    [self.condition unlock];
    
    self.previewImage = image;
    CGRect sourceExtent = image.extent;
    CGFloat previewAspect = _previewBounds.size.width  / _previewBounds.size.height;
    CGRect drawRect = sourceExtent;
    
    if (self.fillMode == QHVCEditFillModeAspectFill)
    {
        //缩放填充
        if (drawRect.size.height > drawRect.size.width)
        {
            drawRect.origin.y += (drawRect.size.height - drawRect.size.width / previewAspect) / 2.0;
            drawRect.size.height = drawRect.size.width / previewAspect;
        }
        else
        {
            drawRect.origin.x += (drawRect.size.width - drawRect.size.height * previewAspect) / 2.0;
            drawRect.size.width = drawRect.size.height * previewAspect;
        }
    }
    else if (self.fillMode == QHVCEditFillModeAspectFit)
    {
        //黑边填充
        if (drawRect.size.height > drawRect.size.width)
        {
            drawRect.origin.x += (drawRect.size.width - drawRect.size.height * previewAspect) / 2.0;
            drawRect.size.width = drawRect.size.height * previewAspect;
        }
        else if (drawRect.size.height < drawRect.size.width)
        {
            drawRect.origin.y += (drawRect.size.height - drawRect.size.width / previewAspect) / 2;
            drawRect.size.height = drawRect.size.width / previewAspect;
        }
        else
        {
            if (previewAspect > 1)
            {
                drawRect.origin.x += (drawRect.size.width - drawRect.size.height * previewAspect) / 2.0;
                drawRect.size.width = drawRect.size.height * previewAspect;
            }
            else
            {
                drawRect.origin.y += (drawRect.size.height - drawRect.size.width / previewAspect) / 2;
                drawRect.size.height = drawRect.size.width / previewAspect;
            }
        }
    }
    
    if (_eaglContext != [EAGLContext currentContext])
    {
        [EAGLContext setCurrentContext:_eaglContext];
    }
    
    [_preview bindDrawable];
    glClearColor(_bgColorRed, _bgColorGreen, _bgColorBlue, _bgColorAlpha);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    if (!self.disable && image && !isnan(drawRect.size.width) && !isnan(drawRect.size.height) && !_stopDrawing)
    {
        [_ciContext drawImage:image inRect:_previewBounds fromRect:drawRect];
        [_preview display];
    }
    glDisable(GL_BLEND);
    self.renderedCount ++;
}
@end

