#include <iostream>
#include <ostream>

#include "dag/dag_factory.h"

namespace Oop {

struct Engine {
  virtual ~Engine() = default;
  virtual void start() = 0;
  virtual unsigned int getSpeed() = 0;
  virtual void setSpeed(unsigned int speed) = 0;
};

struct V6Engine : public Engine {
  ~V6Engine() override { std::cout << "V6 Engine destroyed" << std::endl; }
  void start() override { std::cout << "V6 Engine started" << std::endl; }
  unsigned int getSpeed() override { return m_speed; }
  void setSpeed(unsigned int speed) override { m_speed = speed; }

  unsigned int m_speed = 180;
};

struct I4Engine : public Engine {
  ~I4Engine() override { std::cout << "I4 Engine destroyed" << std::endl; }
  void start() override { std::cout << "I4 Engine started" << std::endl; }
  unsigned int getSpeed() override { return m_speed; }
  void setSpeed(unsigned int speed) override {
    std::cout << "I4 Engine speed set to " << speed << std::endl;
    m_speed = speed;
  }

  unsigned int m_speed = 120;
};

struct Transmission {
  virtual ~Transmission() = default;
  virtual void shift(unsigned int gear) = 0;
};

struct AutoTransmission : public Transmission {
  explicit AutoTransmission(Engine &engine) : m_engine(engine) {
    std::cout << "Auto Transmission created" << std::endl;
  }
  ~AutoTransmission() override { std::cout << "Auto Transmission destroyed" << std::endl; }
  void shift(unsigned int gear) override {
    std::cout << "Auto Transmission shifted to " << gear << std::endl;
    m_engine.setSpeed(gear * 1000);
  }
  Engine &m_engine;
};

struct CVTTransmission : public Transmission {
  explicit CVTTransmission(Engine &engine) : m_engine(engine) {
    std::cout << "CVT Transmission created" << std::endl;
  }
  ~CVTTransmission() override { std::cout << "CVT Transmission destroyed" << std::endl; }
  void shift(unsigned int gear) override {
    std::cout << "CVT Transmission shifted to " << gear << std::endl;
    m_engine.setSpeed(gear * 800);
  }
  Engine &m_engine;
};

struct CarSimulator {
  CarSimulator(Engine &engine, Transmission &transmission)
      : m_engine(engine), m_transmission(transmission) {}
  void start() {
    m_engine.start();
    m_transmission.shift(1);
  }
  Engine &m_engine;
  Transmission &m_transmission;
};

template <typename T>
struct CarSimulatorBlueprint : public dag::Blueprint<T> {
  DAG_TEMPLATE_HELPER();
  CarSimulator &carSimulator() { return make_node<CarSimulator>(engine(), transmission()); }

  virtual Engine &engine() { return i4Engine(); }
  I4Engine &i4Engine() dag_shared { return make_node<I4Engine>(); }
  V6Engine &v6Engine() dag_shared { return make_node<V6Engine>(); }

  virtual Transmission &transmission() { return cvtTransmission(); }
  AutoTransmission &autoTransmission() dag_shared { return make_node<AutoTransmission>(engine()); }
  CVTTransmission &cvtTransmission() dag_shared { return make_node<CVTTransmission>(engine()); }
};

template <typename T>
struct PowerfulCarSimulatorBlueprint : public CarSimulatorBlueprint<T> {
  DAG_TEMPLATE_HELPER();
  Engine &engine() override { return this->v6Engine(); }
  Transmission &transmission() override { return this->autoTransmission(); }
};

}  // namespace Oop

namespace Template {

struct V6Engine {
  ~V6Engine() { std::cout << "V6 Engine destroyed" << std::endl; }
  void start() { std::cout << "V6 Engine started" << std::endl; }
  unsigned int getSpeed() { return m_speed; }
  void setSpeed(unsigned int speed) {
    std::cout << "V6 Engine speed set to " << speed << std::endl;
    m_speed = speed;
  }
  unsigned int m_speed = 180;
};

struct I4Engine {
  ~I4Engine() { std::cout << "I4 Engine destroyed" << std::endl; }
  void start() { std::cout << "I4 Engine started" << std::endl; }
  unsigned int getSpeed() { return m_speed; }
  void setSpeed(unsigned int speed) {
    std::cout << "I4 Engine speed set to " << speed << std::endl;
    m_speed = speed;
  }
  unsigned int m_speed = 120;
};

template <typename Engine>
struct AutoTransmission {
  explicit AutoTransmission(Engine &engine) : m_engine(engine) {
    std::cout << "Auto Transmission created" << std::endl;
  }
  ~AutoTransmission() { std::cout << "Auto Transmission destroyed" << std::endl; }
  void shift(unsigned int gear) {
    std::cout << "Auto Transmission shifted to " << gear << std::endl;
    m_engine.setSpeed(gear * 1000);
  }
  Engine &m_engine;
};

template <typename Engine>
struct CVTTransmission {
  explicit CVTTransmission(Engine &engine) : m_engine(engine) {
    std::cout << "CVT Transmission created" << std::endl;
  }
  ~CVTTransmission() { std::cout << "CVT Transmission destroyed" << std::endl; }
  void shift(unsigned int gear) {
    std::cout << "CVT Transmission shifted to " << gear << std::endl;
    m_engine.setSpeed(gear * 800);
  }
  Engine &m_engine;
};

template <typename Engine, typename Transmission>
struct CarSimulator {
  CarSimulator(Engine &engine, Transmission &transmission)
      : m_engine(engine), m_transmission(transmission) {}
  void start() {
    m_engine.start();
    m_transmission.shift(1);
  }
  Engine &m_engine;
  Transmission &m_transmission;
};

template <typename Derived, typename T>
struct CarSimulatorBlueprint : public dag::Blueprint<T> {
  Derived *self = static_cast<Derived *>(this);
  DAG_TEMPLATE_HELPER();

  auto &carSimulator() { return make_node_t<CarSimulator>(self->engine(), self->transmission()); }
  auto &engine() dag_shared { make_node<I4Engine>(); }
  auto &transmission() dag_shared { return make_node_t<CVTTransmission>(self->engine()); }
};

template <typename T>
struct PowerfulCarSimulatorBlueprint
    : public CarSimulatorBlueprint<PowerfulCarSimulatorBlueprint<T>, T> {
  DAG_TEMPLATE_HELPER();
  auto &engine() dag_shared { return make_node<V6Engine>(); }
  auto &transmission() dag_shared { return make_node_t<AutoTransmission>(engine()); }
};

}  // namespace Template

void run_oop() {
  using namespace Oop;
  dag::DagFactory<CarSimulatorBlueprint> factory;
  dag::unique_ptr<CarSimulator> simulator =
      factory.create([](auto bp) -> auto & { return bp->carSimulator(); });
  simulator->start();
}

void run_template() {
  using namespace Template;
  dag::DagFactory<PowerfulCarSimulatorBlueprint> factory;
  auto simulator = factory.create([](auto bp) -> auto & { return bp->carSimulator(); });
  simulator->start();
}