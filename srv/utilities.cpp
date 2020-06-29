#include "utilities.h"
#include <opencv2/opencv.hpp>
#include <QPolygon>
#include <QImage>
#pragma execution_character_set("utf-8")


void calculateMulThreshold(cv::Mat &src, int *multhreshold)
{
    int height = src.rows;
    int width = src.cols;
    //histogram
    float histogram[256] = { 0 };
    for (int i = 0; i<height; i++)
    {
        uchar*p = src.ptr<uchar>(i);
        for (int j = 0; j<width; j++)
        {
            histogram[*p++]++;
        }
    }
    //normalize histogram:Pi
    int size = height*width;
    for (int i = 0; i<256; i++)
    {
        histogram[i] = histogram[i] / size;
    }
    //average pixel value:u
    double u = 0;
    for (int i = 0; i<256; i++)
    {
        u += i*histogram[i];
    }
    // ///
    double maxVariance = 0;
    double w0 = 0, u0 = 0, w1 = 0, u1 = 0, w2 = 0, u2 = 0;
    double variance = 0;
    for (int k1 = 1; k1 < 255; k1++)
    {
        //求w0,u0;
        w0 = 0; u0 = 0;//!!
        for (int i = 0; i < k1; i++)
        {
            w0 += histogram[i];
            u0 += i*histogram[i];
        }
        if (!w0)
            continue;
        u0 = u0 / w0;

        for (int k2 = k1 + 1; k2 <256; k2++)
        {
            //求w1,u1;
            w1 = 0; u1 = 0;//!!
            for (int i = k1; i < k2; i++)
            {
                w1 += histogram[i];
                u1 += i*histogram[i];
            }
            if (!w1)
                continue;
            u1 = u1 / w1;
            //求w2,u2;
            w2 = 1 - w0 - w1;
            u2 = (u - w0*u0 - w1*u1) / w2;

            variance = w0*(u0 - u)*(u0 - u) + w1*(u1 - u)*(u1 - u) + w2*(u2 - u)*(u2 - u);

            if (variance>maxVariance)
            {
                maxVariance = variance;
                multhreshold[0] = k1;
                multhreshold[1] = k2;
            }
        }
    }
}

QPolygon findCell(const QImage& image, const QPolygon& poly)
{
    cv::Mat src = cv::Mat(image.height(), image.width(), CV_8UC3,
                          const_cast<uchar*>(image.bits()),
                          static_cast<size_t>(image.bytesPerLine()));


    cv::Mat mask(src.size(),CV_8UC1, cv::Scalar(0));
    std::vector<std::vector<cv::Point>> contours({std::vector<cv::Point>()});
    std::transform(poly.begin(), poly.end(), std::back_inserter(contours.front()), [](const QPoint& p){ return cv::Point(p.x(), p.y());});
    polylines(mask, contours, 1, cv::Scalar(255));
    fillPoly(mask, contours, cv::Scalar(255));

    std::vector<cv::Mat> mv;
    split(src, mv);
    cv::Mat mat = mv[0] - mv[1];

    cv::Mat imgROI(mat.rows + 20, mat.cols + 20,CV_8UC1, cv::Scalar(0));
    mat.copyTo(imgROI(cv::Rect(10, 10, mat.cols , mat.rows)), mask);

    cv::Mat imgBinary;

    int thValue[2];
    calculateMulThreshold(imgROI,thValue);
    threshold(imgROI, imgBinary, thValue[0], 255, CV_THRESH_BINARY);

    cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10,10));
    morphologyEx(imgBinary, imgBinary, cv::MORPH_CLOSE, element);

    contours.clear();
    findContours(imgBinary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    QPolygon polygon;
    auto iter = std::max_element(contours.begin(), contours.end(), [](std::vector<cv::Point>& left, std::vector<cv::Point>& right)
    {
        return contourArea(left) < contourArea(right);
    });
    if (iter != contours.end())
    {
        std::transform(iter->begin(), iter->end(), std::back_inserter(polygon), [](const cv::Point& p){
            return QPoint(p.x - 10, p.y - 10);
        });
    }

    return polygon;
}
