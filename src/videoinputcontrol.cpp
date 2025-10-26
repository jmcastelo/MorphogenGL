

#include "videoinputcontrol.h"


#include <QAbstractVideoBuffer>



VideoInput::VideoInput(QCameraDevice cameraDevice, QObject* parent)
    : QObject { parent }
{
    mCamera = new QCamera(cameraDevice);

    mVideoSink = new QVideoSink();

    connect(mVideoSink, &QVideoSink::videoFrameChanged, this, &VideoInput::videoFrameChanged);

    mCaptureSession.setCamera(mCamera);
    mCaptureSession.setVideoSink(mVideoSink);
}



VideoInput::~VideoInput()
{
    if (mCamera->isActive()) {
        mCamera->setActive(false);
    }
    delete mCamera;

    delete mVideoSink;
}



void VideoInput::setCameraActive(bool active)
{
    if ((active && !mCamera->isActive()) || (!active && mCamera->isActive())) {
        mCamera->setActive(active);
    }
}



VideoInputControl::VideoInputControl(QObject* parent)
    : QObject { parent }
{
    connect(&mMediaDevices, &QMediaDevices::videoInputsChanged, this, &VideoInputControl::setVideoInputs);
    setVideoInputs();
}



VideoInputControl::~VideoInputControl()
{
    foreach (auto videoInput, mVideoInMap) {
        delete videoInput;
    }
}



QList<QString> VideoInputControl::cameraDescriptions()
{
    return mCameraDescMap.values();
}



QByteArray VideoInputControl::cameraId(int index)
{
    QList<QByteArray> cameraIds = mCameraDescMap.keys();
    QByteArray camId = cameraIds.at(index);

    return camId;
}


void VideoInputControl::useCamera(int index)
{
    QList<QByteArray> cameraIds = mCameraDescMap.keys();
    QByteArray camId = cameraIds.at(index);

    mNumUsedCamerasMap[camId]++;

    if (mNumUsedCamerasMap[camId] == 1)
    {
        emit cameraUsed(camId);
        mVideoInMap.value(camId)->setCameraActive(true);
    }

    emit numUsedCamerasChanged();
}



void VideoInputControl::unuseCamera(QByteArray camId)
{
    if (mNumUsedCamerasMap.contains(camId))
    {
        mNumUsedCamerasMap[camId]--;

        if (mNumUsedCamerasMap[camId] == 0)
        {
            mVideoInMap.value(camId)->setCameraActive(false);
            emit cameraUnused(camId);
        }

        emit numUsedCamerasChanged();
    }
}



void VideoInputControl::setVideoInputs()
{
    const QList<QCameraDevice> videoDevices = QMediaDevices::videoInputs();

    QMap<QByteArray, QCameraDevice> deviceMap;

    foreach (const QCameraDevice &device, videoDevices) {
        deviceMap.insert(device.id(), device);
    }

    for (auto [id, videoInput] : mVideoInMap.asKeyValueRange())
    {
        if (!deviceMap.contains(id))
        {
            disconnect(videoInput, &VideoInput::videoFrameChanged, nullptr, nullptr);

            unuseCamera(id);

            delete videoInput;
            mVideoInMap.remove(id);
            mCameraDescMap.remove(id);
        }
        else
        {
            deviceMap.remove(id);
        }
    }

    for (auto [id, device] : deviceMap.asKeyValueRange())
    {
        VideoInput* videoInput = new VideoInput(device);
        mVideoInMap.insert(id, videoInput);
        mCameraDescMap.insert(id, device.description());

        connect(videoInput, &VideoInput::videoFrameChanged, [=, this](const QVideoFrame& videoFrame) {
            emit newFrameImage(id, videoFrame.toImage());
        });
    }
}
