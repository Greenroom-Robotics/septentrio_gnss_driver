import os
import launch
from launch_ros.actions import Node

os.environ['RCUTILS_CONSOLE_OUTPUT_FORMAT'] = '{time}: [{name}] [{severity}]\t{message}'

def generate_launch_description():
    
    tf_imu = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        arguments = "0 0 0 0 0 0 base_link imu".split(' ')
    )

    tf_gnss = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        arguments = "0 0 0 0 0 0 imu gnss".split(' ')
    )

    tf_vsm = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        arguments = "0 0 0 0 0 0 imu vsm".split(' ')
    )

    tf_aux1 = Node(
        package="tf2_ros",
        executable="static_transform_publisher",
        arguments = "0 0 0 0 0 0 imu aux1".split(' ')
    )

    parameters_list = [
        {
            "covariance_threshold": 50.0,
            "device": "tcp://10.27.1.102:28784",
            "configure_rx": True,
            "keep_open": True,
            "frame_id": "septentrio_gnss",
            "imu_frame_id": "imu",
            "poi_frame_id": "base_link",
            "vsm_frame_id": "vsm",
            "aux1_frame_id": "aux1",
            "vehicle_frame_id": "base_link",
            "local_frame_id": "odom",
            "insert_local_frame": True,
            "lock_utm_zone": True,
            "use_ros_axis_orientation": True,
            "receiver_type": "ins",
            "multi_antenna": True,
            "datum": "Default",
            "att_offset": {"heading": 90.0, "pitch": 0.0},
            "polling_period.pvt": 100,
            "gnss_error_level": 1,
            "gnss_warn_level": 4,
            "covariance_threshold": 50.0,
            "publish": {
                "geopose_stamped": True,
                "twist_flu_stamped": True,
                "diagnostics": True,
            },
            "ins_spatial_config": {
                "imu_orientation": {"theta_x": 0.0, "theta_y": 0.0, "theta_z": 0.0},
                "poi_lever_arm": {"delta_x": 0.0, "delta_y": 0.83, "delta_z": 0.0},
                "ant_lever_arm": {"x": 0.0, "y": -0.5, "z": 0.0},
                "vsm_lever_arm": {"vsm_x": 0.0, "vsm_y": 0.0, "vsm_z": 0.0},
            },
            "ins_initial_heading": "auto",
            "ins_std_dev_mask": {"att_std_dev": 5.0, "pos_std_dev": 10.0},
            "ins_use_poi": False,
            "ins_vsm": {
                "ros": {
                    "source": "",
                    "config": [False, False, False],
                    "variances_by_parameter": False,
                    "variances": [0.0, 0.0, 0.0],
                },
                "ip_server": {"id": "", "port": 0, "keep_open": True},
                "serial": {"port": "", "baud_rate": 115200, "keep_open": True},
            },
            "activate_debug_log": False,
        }
    ]
    
    node = Node(
        package='septentrio_gnss_driver',
        executable='septentrio_gnss_driver_node',
        name='septentrio_ins',
        emulate_tty=True,
        sigterm_timeout = '20',
        parameters=parameters_list,
        namespace='vessel_1',
        remappings=[
            ("/diagnostics", "gama/diagnostics"),
            ("geopose", "geopose"),
            ("twist_flu", "twist"),
        ],
        ros_arguments=["--log-level", "info", "--log-level", "rcl:=warn"],
    )

    return launch.LaunchDescription([node, tf_imu, tf_gnss, tf_vsm, tf_aux1])
