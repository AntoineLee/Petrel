//
//  QHVCEditSetOutputParamsVC.m
//  QHVideoCloudToolSet
//
//  Created by liyue-g on 2018/11/14.
//  Copyright © 2018年 yangkui. All rights reserved.
//

#import "QHVCEditSetOutputParamsVC.h"
#import "QHVCEditReorderVC.h"
#import "QHVCEditMediaEditorConfig.h"
#import "QHVCEditPrefs.h"
#import "UIView+Toast.h"

@interface QHVCEditSetOutputParamsVC () <UIPickerViewDelegate, UIPickerViewDataSource>
@property (weak, nonatomic) IBOutlet NSLayoutConstraint *topConstraint;
@property (weak, nonatomic) IBOutlet UIButton *outputSizeBtn;
@property (weak, nonatomic) IBOutlet UIButton *fpsBtn;
@property (weak, nonatomic) IBOutlet UILabel *bitrateLabel;
@property (weak, nonatomic) IBOutlet UIView *outputSizePickerView;
@property (weak, nonatomic) IBOutlet UIPickerView *outputSizePicker;
@property (weak, nonatomic) IBOutlet UIView *outputFpsPickerView;
@property (weak, nonatomic) IBOutlet UIPickerView *outputFpsPicker;
@property (weak, nonatomic) IBOutlet UITextField *customWidthTextfield;
@property (weak, nonatomic) IBOutlet UITextField *customHeightTextfield;

@property (nonatomic, retain) NSArray* photoItems;
@property (nonatomic, retain) NSArray* outputSizeArray;
@property (nonatomic, retain) NSArray* fpsArray;
@property (nonatomic, assign) NSInteger curOutputWidth;
@property (nonatomic, assign) NSInteger curOutputHeight;
@property (nonatomic, assign) NSInteger curOutputFps;
@property (nonatomic, assign) NSInteger curBitrate;

@end

@implementation QHVCEditSetOutputParamsVC

#pragma mark - Life Circle Methods

- (instancetype)initWithItems:(NSArray<QHVCPhotoItem *> *)items
{
    if (!(self = [super init]))
    {
        return nil;
    }
    
    self.photoItems = items;
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setTitle:@"设置输出参数"];
    [self.topConstraint setConstant:[self topBarHeight]];
    
    self.outputSizeArray = @[@[@(640), @(360)],
                             @[@(720), @(480)],
                             @[@(1280), @(720)],
                             @[@(1920), @(1080)]];
    self.fpsArray = @[@(15), @(24), @(30)];
    self.curOutputFps = 30;
    self.curOutputWidth = 1280;
    self.curOutputHeight = 720;
    self.curBitrate = 4.5 * 1000 * 1000;
}

#pragma mark - Event Methods

- (void)nextAction:(UIButton *)btn
{
    if (self.curOutputWidth <=0 || self.curOutputHeight <= 0)
    {
        [self.view makeToast:@"输出分辨率必须大于0"];
        return;
    }
    
    [[QHVCEditMediaEditorConfig sharedInstance] setOutputSize:CGSizeMake(self.curOutputWidth, self.curOutputHeight)];
    [[QHVCEditMediaEditorConfig sharedInstance] setOutputFps:self.curOutputFps];
    [[QHVCEditMediaEditorConfig sharedInstance] setOutputVideoBitrate:self.curBitrate];
    QHVCEditReorderVC *vc = [[QHVCEditReorderVC alloc] initWithItems:self.photoItems];
    [self.navigationController pushViewController:vc animated:YES];
}

- (IBAction)clickedOutputSizeBtn:(UIButton *)sender
{
    [self.outputSizePickerView setHidden:NO];
    [self.outputFpsPickerView setHidden:YES];
    
    if ([self.customWidthTextfield isFirstResponder])
    {
        [self.customWidthTextfield resignFirstResponder];
    }
    
    if ([self.customHeightTextfield isFirstResponder])
    {
        [self.customHeightTextfield resignFirstResponder];
    }
}

- (IBAction)clickedOutputSizeConfirmBtn:(id)sender
{
    if ([self.outputSizePickerView isHidden])
    {
        return;
    }
    
    NSInteger row = [self.outputSizePicker selectedRowInComponent:0];
    NSArray* size = [self.outputSizeArray objectAtIndex:row];
    NSInteger width = [size[0] integerValue];
    NSInteger height = [size[1] integerValue];

    [self updateOutputSizeBtnTitle:width height:height];
    [self.customWidthTextfield setText:@""];
    [self.customHeightTextfield setText:@""];
    [self.outputSizePickerView setHidden:YES];
}

- (IBAction)clickedFPSBtn:(UIButton *)sender
{
    [self.outputFpsPickerView setHidden:NO];
    [self.outputSizePickerView setHidden:YES];
    
    if ([self.customWidthTextfield isFirstResponder])
    {
        [self.customWidthTextfield resignFirstResponder];
    }
    
    if ([self.customHeightTextfield isFirstResponder])
    {
        [self.customHeightTextfield resignFirstResponder];
    }
}

- (IBAction)clickedOutputFpsConfirmBtn:(id)sender
{
    if ([self.outputFpsPickerView isHidden])
    {
        return;
    }
    
    NSInteger row = [self.outputFpsPicker selectedRowInComponent:0];
    NSNumber* fps = [self.fpsArray objectAtIndex:row];
    [self.fpsBtn setTitle:[NSString stringWithFormat:@"%@", fps] forState:UIControlStateNormal];
    [self.outputFpsPickerView setHidden:YES];
    self.curOutputFps = [fps integerValue];
}

- (IBAction)onVideoBitrateSliderValueChanged:(UISlider *)sender
{
    self.curBitrate = sender.value * 1000 * 1000;
    [self.bitrateLabel setText:[NSString stringWithFormat:@"%.1fMbps", sender.value]];
}

#pragma mark - Picker View Methods

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    if (pickerView == self.outputSizePicker)
    {
        return self.outputSizeArray.count;
    }
    else if (pickerView == self.outputFpsPicker)
    {
        return self.fpsArray.count;
    }
    return 0;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component
{
    return 40;
}

- (nullable NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    if (pickerView == self.outputSizePicker)
    {
        if ([self.outputSizeArray count] > row)
        {
            NSInteger width = [self.outputSizeArray[row][0] integerValue];
            NSInteger height = [self.outputSizeArray[row][1] integerValue];
            NSString* title = [NSString stringWithFormat:@"%ldx%ld", (long)width, (long)height];
            return title;
        }
    }
    else if (pickerView == self.outputFpsPicker)
    {
        if ([self.fpsArray count] > row)
        {
            NSInteger fps = [self.fpsArray[row] integerValue];
            NSString* title = [NSString stringWithFormat:@"%ld", fps];
            return title;
        }
    }
    return @"";
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    if (pickerView == self.outputSizePicker)
    {
        NSArray* size = [self.outputSizeArray objectAtIndex:row];
        NSInteger width = [size[0] integerValue];
        NSInteger height = [size[1] integerValue];
        [self updateOutputSizeBtnTitle:width height:height];
    }
    else if (pickerView == self.outputFpsPicker)
    {
        NSNumber* fps = [self.fpsArray objectAtIndex:row];
        [self.fpsBtn setTitle:[NSString stringWithFormat:@"%@", fps] forState:UIControlStateNormal];
    }
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    [self.outputSizePickerView setHidden:YES];
    [self.outputFpsPickerView setHidden:YES];
    return YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    NSString* widthText = self.customWidthTextfield.text;
    NSString* heightText = self.customHeightTextfield.text;
    NSInteger width = [widthText integerValue];
    NSInteger height = [heightText integerValue];
    
    if (width > 0 && height > 0)
    {
        [self updateOutputSizeBtnTitle:width height:height];
    }
    [textField resignFirstResponder];
    return YES;
}

- (void)updateOutputSizeBtnTitle:(NSInteger)width height:(NSInteger)height
{
    NSString* title = [NSString stringWithFormat:@"%ldx%ld", (long)width, (long)height];
    self.curOutputWidth = width;
    self.curOutputHeight = height;
    [self.outputSizeBtn setTitle:title forState:UIControlStateNormal];
}

@end
