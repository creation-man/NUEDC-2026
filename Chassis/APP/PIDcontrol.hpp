#ifndef PID_HPP
#define PID_HPP

/**
 * @brief 增量式 PID 控制类
 */
class PID {
public:
    /**
     * @brief PID 构造函数
     * @param kp 比例系数
     * @param ki 积分系数
     * @param kd 微分系数
     * @param max_out 输出限幅（例如 PWM 的最大值）
     */
    PID(float kp, float ki, float kd, float max_out);

    /**
     * @brief 设置目标值
     * @param target 目标速度 (RPM 或 脉冲增量)
     */
    void setTarget(float target);

    /**
     * @brief PID 计算核心
     * @param measured 当前测量值（反馈值）
     * @return float 控制输出量（PWM）
     */
    float calculate(float measured);

    /**
     * @brief 重置 PID 内部误差状态
     */
    void reset();

    // 允许动态修改 PID 参数（用于调参）
    void updateParams(float kp, float ki, float kd);

private:
    float _kp, _ki, _kd;      // PID 参数
    float _max_out;           // 输出限幅
    float _target;            // 目标值
    float _measured;          // 测量值
    
    float _err;               // 当前误差 e(k)
    float _last_err;          // 上一次误差 e(k-1)
    float _prev_err;          // 上上次误差 e(k-2)
    float _output;            // 当前累积输出
    float _integral; // <--- 新增这一行
};

#endif // PID_HPP