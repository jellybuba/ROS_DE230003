from setuptools import find_packages, setup
from glob import glob

package_name = 'mobile_robot'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch',glob('launch/*')),
        ('share/' + package_name + '/model',glob('model/*')),
        ('share/' + package_name + '/parameters',glob('parameters/*')),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Dmitrijs Tolstihs',
    maintainer_email='dtolstyh413@gmail.com',
    description='TODO: Package description',
    license='MIT',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
        ],
    },
)
