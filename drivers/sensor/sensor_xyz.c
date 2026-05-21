#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <math.h>

struct sensor_xyz_data
{
    float x; 
    float y;
    float z;
    int64_t t_ms; 
};

static struct sensor_xyz_data data;

static void sample_work_handler(struct k_work *work);
K_WORK_DEFINE(sample_work, sample_work_handler);

static struct k_timer sample_timer;

static void timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_work_submit(&sample_work);
}

static void sample_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);
    static float t = 0;

    data.x = sinf(t);
    data.y = cosf(t);
    data.z = 9.80665f;
    data.t_ms = k_uptime_get();

    t += 0.1f;
}

static int sensor_xyz_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(chan);
    return 0;
}

static void float_to_sensor_value(float val, struct sensor_value *out)
{
    out->val1 = (int32_t)val;
    out->val2 = (int32_t)((val - out->val1) * 1000000); 
}

static int sensor_xyz_channel_get(const struct device *dev, enum sensor_channel chan, struct sensor_value *val)
{
    ARG_UNUSED(dev);

    switch(chan) {
    case SENSOR_CHAN_ACCEL_X:
        float_to_sensor_value(data.x, val);
        break;

    case SENSOR_CHAN_ACCEL_Y:
        float_to_sensor_value(data.y, val);
        break;

    case SENSOR_CHAN_ACCEL_Z:
        float_to_sensor_value(data.z, val);
        break;

    case SENSOR_CHAN_ACCEL_XYZ:
        float_to_sensor_value(data.x, &val[0]);
        float_to_sensor_value(data.y, &val[1]);
        float_to_sensor_value(data.z, &val[2]);
        break;

    default:
        return -ENOTSUP;
    }

    return 0;
}

static const struct sensor_driver_api sensor_xyz_api = {
    .sample_fetch = sensor_xyz_sample_fetch,
    .channel_get = sensor_xyz_channel_get,
};

static int sensor_xyz_init(const struct device *dev)
{
    ARG_UNUSED(dev);

    int period_ms = 1000 / CONFIG_SENSOR_XYZ_RATE_HZ;

    k_timer_init(&sample_timer, timer_handler, NULL);
    k_timer_start(&sample_timer, K_MSEC(period_ms), K_MSEC(period_ms));

    return 0;
}

DEVICE_DEFINE (sensor_xyz, "FAKE_XYZ", sensor_xyz_init, NULL,
                NULL, NULL, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,
                &sensor_xyz_api);