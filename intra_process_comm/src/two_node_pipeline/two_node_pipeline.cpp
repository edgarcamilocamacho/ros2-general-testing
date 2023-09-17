// Copyright 2015 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;

// Node that produces messages.
class Producer : public rclcpp::Node
{
  public:

    Producer(const std::string & name, const std::string & output)
          : Node(name, rclcpp::NodeOptions().use_intra_process_comms(true)),
            name_(name)
      {
        pub_ = this->create_publisher<std_msgs::msg::Int32>(output, 10);
        timer_ = this->create_wall_timer(
            1s, 
            std::bind(&Producer::cb_timer_pub, this)
          );
      }

  private:

    void cb_timer_pub() {
        static int32_t count = 0; 
        std_msgs::msg::Int32::UniquePtr msg(new std_msgs::msg::Int32());
        msg->data = count++;
        printf(
          "\n[%s] Publish. Value %d Address 0x%" PRIXPTR "\n",
          name_.c_str(), 
          msg->data,
          reinterpret_cast<std::uintptr_t>(msg.get())
        );
        pub_->publish(std::move(msg));
      }

    std::string name_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;

};

class Consumer : public rclcpp::Node
{
  public:

    Consumer(
        const std::string & name, 
        const std::string & input, 
        int callback_type=0 )
          : Node(name, rclcpp::NodeOptions().use_intra_process_comms(true)),
            name_(name)
      {

        if(callback_type==0){
          sub_ = this->create_subscription<std_msgs::msg::Int32>(
            input, 10,
            std::bind(&Consumer::cb_sub_common, this, _1)
          );
        }else if(callback_type==1){
          sub_ = this->create_subscription<std_msgs::msg::Int32>(
            input, 10,
            std::bind(&Consumer::cb_sub_uniqueptr, this, _1)
          );
        }else if(callback_type==2){
          sub_ = this->create_subscription<std_msgs::msg::Int32>(
            input, 10,
            std::bind(&Consumer::cb_sub_sharedptr, this, _1)
          );
        }

      }

  private:

    void cb_sub_common(const std_msgs::msg::Int32 & msg) {
        printf(
            "  [%s.Common] Received. Value: %d\n",
            name_.c_str(),
            msg.data
          );
      }

    void cb_sub_uniqueptr(std_msgs::msg::Int32::UniquePtr msg) {
        printf(
            "  [%s.UniquePtr] Received. Value: %d  Address: 0x%" PRIXPTR "\n",
            name_.c_str(), 
            msg->data,
            reinterpret_cast<std::uintptr_t>(msg.get())
          );
      }

    void cb_sub_sharedptr(std_msgs::msg::Int32::SharedPtr msg) {
        printf(
            "  [%s.SharedPtr] Received. Value: %d  Address: 0x%" PRIXPTR "\n", 
            name_.c_str(),
            msg->data,
            reinterpret_cast<std::uintptr_t>(msg.get())
          );
      }

    std::string name_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr sub_;

};

int main(int argc, char * argv[])
{
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  rclcpp::init(argc, argv);
  rclcpp::executors::SingleThreadedExecutor executor;
  // rclcpp::executors::MultiThreadedExecutor executor;

  auto producer = std::make_shared<Producer>("producer", "number");
  // auto consumer_common_1 = std::make_shared<Consumer>("consumer_common_1", "number", 0);
  // auto consumer_common_2 = std::make_shared<Consumer>("consumer_common_2", "number", 0);
  auto consumer_unique_1 = std::make_shared<Consumer>("consumer_unique_1", "number", 1);
  // auto consumer_unique_2 = std::make_shared<Consumer>("consumer_unique_2", "number", 1);
  // auto consumer_shared_1 = std::make_shared<Consumer>("consumer_shared_1", "number", 2);
  // auto consumer_shared_2 = std::make_shared<Consumer>("consumer_shared_2", "number", 2);

  executor.add_node(producer);
  // executor.add_node(consumer_common_1);
  // executor.add_node(consumer_common_2);
  executor.add_node(consumer_unique_1);
  // executor.add_node(consumer_unique_2);
  // executor.add_node(consumer_shared_1);
  // executor.add_node(consumer_shared_2);
  executor.spin();

  rclcpp::shutdown();

  return 0;
}
